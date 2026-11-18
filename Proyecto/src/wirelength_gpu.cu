#include <float.h>
#include <string.h>
#include "gpu_internal.h"

// Wirelength en la GPU. En el avance anterior era un hilo por red leyendo los
// Pin de la netlist tal cual; el perfilado mostro dos problemas: los accesos a
// memoria de un warp caian dispersos (cada hilo leia una red distinta, lejos
// de la del vecino) y unas pocas redes enormes (newblue1 tiene una de mas de
// 12000 pines) dejaban todo el kernel esperando a un solo hilo. Ahora:
//  - las redes de menos de BIG_NET pines siguen con un hilo por red, pero
//    ordenadas por grado (los hilos de un warp hacen casi el mismo trabajo) y
//    con sus pines intercalados por warp: el pin k de los 32 hilos de un warp
//    esta en 32 slots seguidos, asi cada lectura del warp es una sola
//    transaccion de 128 bytes;
//  - las redes de BIG_NET pines o mas se reparten entre todo un bloque.
#define BIG_NET 64
#define WARP 32

// Posicion de cada slot, una vez por iteracion y con un hilo por slot. Asi la
// posicion de la celda, que esta dispersa en v, se busca una sola vez por pin
// y no en cada una de las tres pasadas sobre la red.
__global__ void pin_pos_kernel(int num_slots, int n, const int *__restrict__ slot_cell,
                               const float *__restrict__ slot_dx,
                               const float *__restrict__ slot_dy,
                               const float *__restrict__ v, float *px, float *py)
{
    int s = blockIdx.x * blockDim.x + threadIdx.x;
    if (s >= num_slots)
        return;
    int c = slot_cell[s];
    if (c < 0)
        return;   // relleno del final de un warp, nadie lo lee
    px[s] = __ldg(&v[c]) + slot_dx[s];
    py[s] = __ldg(&v[n + c]) + slot_dy[s];
}

// derivada del WA de una red respecto a una coordenada de uno de sus pines
__device__ __forceinline__ float wa_grad(float x, float lo, float hi, float sp, float sm,
                                         float mp, float mm, float inv_gamma)
{
    float ap = expf((x - hi) * inv_gamma), am = expf((lo - x) * inv_gamma);
    return ap / sp * (1 + (x - hi - mp) * inv_gamma) - am / sm * (1 - (x - lo - mm) * inv_gamma);
}

// Un hilo por red chica. Lo unico compartido entre redes es el gradiente de
// una celda que esta en varias, que se acumula con atomicAdd.
__global__ void wa_small_kernel(int num_small, const int *__restrict__ warp_base,
                                const int *__restrict__ small_deg,
                                const float *__restrict__ px, const float *__restrict__ py,
                                const int *__restrict__ slot_cell, int n, float inv_gamma,
                                float *grad, float *partial)
{
    __shared__ float block_sum[GPU_THREADS];
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    float wa = 0.0f;

    if (i < num_small) {
        int deg = small_deg[i];
        int first = warp_base[i / WARP] + i % WARP;
        const float *qx = px + first, *qy = py + first;

        float xlo = FLT_MAX, xhi = -FLT_MAX, ylo = FLT_MAX, yhi = -FLT_MAX;
        for (int k = 0; k < deg; k++) {
            float x = qx[k * WARP], y = qy[k * WARP];
            xlo = fminf(xlo, x);
            xhi = fmaxf(xhi, x);
            ylo = fminf(ylo, y);
            yhi = fmaxf(yhi, y);
        }

        // la CPU lleva estas sumas en double; aca en float se miden desde el
        // extremo que domina cada lado (el + desde el maximo, el - desde el
        // minimo) para que un pin muy lejos del resto no deje la resta del
        // gradiente sin precision. Es la misma cuenta, solo cambia el origen.
        float sxp = 0, wxp = 0, sxm = 0, wxm = 0;
        float syp = 0, wyp = 0, sym = 0, wym = 0;
        for (int k = 0; k < deg; k++) {
            float x = qx[k * WARP], y = qy[k * WARP];
            float axp = expf((x - xhi) * inv_gamma), axm = expf((xlo - x) * inv_gamma);
            float ayp = expf((y - yhi) * inv_gamma), aym = expf((ylo - y) * inv_gamma);
            sxp += axp;
            wxp += (x - xhi) * axp;
            sxm += axm;
            wxm += (x - xlo) * axm;
            syp += ayp;
            wyp += (y - yhi) * ayp;
            sym += aym;
            wym += (y - ylo) * aym;
        }
        float mxp = wxp / sxp, mxm = wxm / sxm;
        float myp = wyp / syp, mym = wym / sym;
        wa = ((xhi - xlo) + mxp - mxm) + ((yhi - ylo) + myp - mym);

        if (grad) {
            const int *qc = slot_cell + first;
            for (int k = 0; k < deg; k++) {
                int c = qc[k * WARP];
                atomicAdd(&grad[c], wa_grad(qx[k * WARP], xlo, xhi, sxp, sxm, mxp, mxm, inv_gamma));
                atomicAdd(&grad[n + c], wa_grad(qy[k * WARP], ylo, yhi, syp, sym, myp, mym, inv_gamma));
            }
        }
    }

    // suma del WA de las redes del bloque; el host suma los parciales
    block_sum[threadIdx.x] = wa;
    __syncthreads();
    for (unsigned s = blockDim.x / 2; s > 0; s >>= 1) {
        if (threadIdx.x < s)
            block_sum[threadIdx.x] += block_sum[threadIdx.x + s];
        __syncthreads();
    }
    if (threadIdx.x == 0)
        partial[blockIdx.x] = block_sum[0];
}

// Una red grande por bloque: cada hilo recorre los pines t, t+blockDim, ...
// y los extremos y las sumas de la red se juntan en memoria compartida.
__global__ void wa_big_kernel(const int *__restrict__ big_start, const int *__restrict__ big_deg,
                              const float *__restrict__ px, const float *__restrict__ py,
                              const int *__restrict__ slot_cell, int n, float inv_gamma,
                              float *grad, float *partial)
{
    __shared__ float red[8][GPU_THREADS];
    unsigned t = threadIdx.x;
    int first = big_start[blockIdx.x], last = first + big_deg[blockIdx.x];

    float xlo = FLT_MAX, xhi = -FLT_MAX, ylo = FLT_MAX, yhi = -FLT_MAX;
    for (int k = first + t; k < last; k += blockDim.x) {
        xlo = fminf(xlo, px[k]);
        xhi = fmaxf(xhi, px[k]);
        ylo = fminf(ylo, py[k]);
        yhi = fmaxf(yhi, py[k]);
    }
    red[0][t] = xlo;
    red[1][t] = xhi;
    red[2][t] = ylo;
    red[3][t] = yhi;
    __syncthreads();
    for (unsigned s = blockDim.x / 2; s > 0; s >>= 1) {
        if (t < s) {
            red[0][t] = fminf(red[0][t], red[0][t + s]);
            red[1][t] = fmaxf(red[1][t], red[1][t + s]);
            red[2][t] = fminf(red[2][t], red[2][t + s]);
            red[3][t] = fmaxf(red[3][t], red[3][t + s]);
        }
        __syncthreads();
    }
    xlo = red[0][0];
    xhi = red[1][0];
    ylo = red[2][0];
    yhi = red[3][0];
    __syncthreads();   // todos leyeron los extremos antes de reusar red[]

    // las mismas ocho sumas de wa_small_kernel, en el mismo orden
    float acc[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    for (int k = first + t; k < last; k += blockDim.x) {
        float x = px[k], y = py[k];
        float axp = expf((x - xhi) * inv_gamma), axm = expf((xlo - x) * inv_gamma);
        float ayp = expf((y - yhi) * inv_gamma), aym = expf((ylo - y) * inv_gamma);
        acc[0] += axp;
        acc[1] += (x - xhi) * axp;
        acc[2] += axm;
        acc[3] += (x - xlo) * axm;
        acc[4] += ayp;
        acc[5] += (y - yhi) * ayp;
        acc[6] += aym;
        acc[7] += (y - ylo) * aym;
    }
    for (int j = 0; j < 8; j++)
        red[j][t] = acc[j];
    __syncthreads();
    for (unsigned s = blockDim.x / 2; s > 0; s >>= 1) {
        if (t < s)
            for (int j = 0; j < 8; j++)
                red[j][t] += red[j][t + s];
        __syncthreads();
    }
    float sxp = red[0][0], sxm = red[2][0], syp = red[4][0], sym = red[6][0];
    float mxp = red[1][0] / sxp, mxm = red[3][0] / sxm;
    float myp = red[5][0] / syp, mym = red[7][0] / sym;
    if (t == 0)
        partial[blockIdx.x] = ((xhi - xlo) + mxp - mxm) + ((yhi - ylo) + myp - mym);

    if (grad) {
        for (int k = first + t; k < last; k += blockDim.x) {
            int c = slot_cell[k];
            atomicAdd(&grad[c], wa_grad(px[k], xlo, xhi, sxp, sxm, mxp, mxm, inv_gamma));
            atomicAdd(&grad[n + c], wa_grad(py[k], ylo, yhi, syp, sym, myp, mym, inv_gamma));
        }
    }
}

// Acomoda la netlist para los kernels de arriba y la sube. Las redes de grado
// menor a 2 no aportan nada (ni WA ni gradiente) y quedan fuera.
void wirelength_gpu_init(GpuPlacer *g, const Netlist *nl)
{
    int ne = nl->num_nets;

    // redes chicas ordenadas por grado con un conteo; las grandes aparte
    int count[BIG_NET];
    memset(count, 0, sizeof count);
    int num_big = 0;
    for (int e = 0; e < ne; e++) {
        int d = nl->nets[e].degree;
        if (d >= BIG_NET)
            num_big++;
        else if (d >= 2)
            count[d]++;
    }
    int num_small = 0;
    for (int d = 2; d < BIG_NET; d++) {
        int c = count[d];
        count[d] = num_small;   // ahora es donde empiezan las de grado d
        num_small += c;
    }
    int *order = (int *)malloc((size_t)(num_small + num_big + 1) * sizeof(int));
    int *big = order + num_small;
    int nb = 0;
    for (int e = 0; e < ne; e++) {
        int d = nl->nets[e].degree;
        if (d >= BIG_NET)
            big[nb++] = e;
        else if (d >= 2)
            order[count[d]++] = e;
    }

    // cada warp de redes chicas reserva 32 slots por cada pin de su red mas
    // grande (la ultima, porque van ordenadas); el relleno solo aparece en los
    // warps donde cambia el grado. Despues van las redes grandes, seguidas.
    int num_warps = (num_small + WARP - 1) / WARP;
    int *warp_base = (int *)malloc((size_t)(num_warps + 1) * sizeof(int));
    int *small_deg = (int *)malloc((size_t)(num_small + 1) * sizeof(int));
    int *big_start = (int *)malloc((size_t)(num_big + 1) * sizeof(int));
    int *big_deg = (int *)malloc((size_t)(num_big + 1) * sizeof(int));
    int slots = 0;
    for (int w = 0; w < num_warps; w++) {
        int last = (w + 1) * WARP < num_small ? (w + 1) * WARP - 1 : num_small - 1;
        warp_base[w] = slots;
        slots += WARP * nl->nets[order[last]].degree;
    }
    for (int j = 0; j < num_big; j++) {
        big_start[j] = slots;
        big_deg[j] = nl->nets[big[j]].degree;
        slots += big_deg[j];
    }

    int *cell = (int *)malloc((size_t)(slots + 1) * sizeof(int));
    float *dx = (float *)calloc((size_t)slots + 1, sizeof(float));
    float *dy = (float *)calloc((size_t)slots + 1, sizeof(float));
    for (int s = 0; s <= slots; s++)
        cell[s] = -1;
    for (int i = 0; i < num_small; i++) {
        const Net *net = &nl->nets[order[i]];
        int first = warp_base[i / WARP] + i % WARP;
        small_deg[i] = net->degree;
        for (int k = 0; k < net->degree; k++) {
            const Pin *p = &nl->pins[net->pin_start + k];
            cell[first + k * WARP] = p->cell;
            dx[first + k * WARP] = p->dx;
            dy[first + k * WARP] = p->dy;
        }
    }
    for (int j = 0; j < num_big; j++) {
        const Net *net = &nl->nets[big[j]];
        for (int k = 0; k < net->degree; k++) {
            const Pin *p = &nl->pins[net->pin_start + k];
            cell[big_start[j] + k] = p->cell;
            dx[big_start[j] + k] = p->dx;
            dy[big_start[j] + k] = p->dy;
        }
    }

    g->num_slots = slots;
    g->num_small = num_small;
    g->num_big = num_big;
    g->slot_cell = (int *)gpu_upload(cell, (size_t)(slots + 1) * sizeof(int));
    g->slot_dx = (float *)gpu_upload(dx, (size_t)(slots + 1) * sizeof(float));
    g->slot_dy = (float *)gpu_upload(dy, (size_t)(slots + 1) * sizeof(float));
    g->warp_base = (int *)gpu_upload(warp_base, (size_t)(num_warps + 1) * sizeof(int));
    g->small_deg = (int *)gpu_upload(small_deg, (size_t)(num_small + 1) * sizeof(int));
    g->big_start = (int *)gpu_upload(big_start, (size_t)(num_big + 1) * sizeof(int));
    g->big_deg = (int *)gpu_upload(big_deg, (size_t)(num_big + 1) * sizeof(int));
    CUDA_CHECK(cudaMalloc(&g->px, (size_t)(slots + 1) * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&g->py, (size_t)(slots + 1) * sizeof(float)));

    free(order);
    free(warp_base);
    free(small_deg);
    free(big_start);
    free(big_deg);
    free(cell);
    free(dx);
    free(dy);
}

void wirelength_gpu_free(GpuPlacer *g)
{
    cudaFree(g->slot_cell);
    cudaFree(g->slot_dx);
    cudaFree(g->slot_dy);
    cudaFree(g->px);
    cudaFree(g->py);
    cudaFree(g->warp_base);
    cudaFree(g->small_deg);
    cudaFree(g->big_start);
    cudaFree(g->big_deg);
}

int wirelength_gpu_partials(const GpuPlacer *g)
{
    return (g->num_small + GPU_THREADS - 1) / GPU_THREADS + g->num_big;
}

// encola en s_wl el calculo sobre la v ya subida, sin esperar
void wirelength_enqueue(GpuPlacer *g, float gamma, int with_grad)
{
    size_t bytes = 2 * (size_t)g->num_cells * sizeof(float);
    int slot_blocks = (g->num_slots + GPU_THREADS - 1) / GPU_THREADS;
    int small_blocks = (g->num_small + GPU_THREADS - 1) / GPU_THREADS;
    float inv_gamma = 1.0f / gamma;
    float *dgrad = with_grad ? g->grad : NULL;
    cudaStream_t s = g->s_wl;
    g->wl_grad = with_grad;

    if (with_grad)
        CUDA_CHECK(cudaMemsetAsync(g->grad, 0, bytes, s));
    if (slot_blocks > 0) {
        pin_pos_kernel<<<slot_blocks, GPU_THREADS, 0, s>>>(g->num_slots, g->num_cells,
                                                           g->slot_cell, g->slot_dx, g->slot_dy,
                                                           g->v, g->px, g->py);
        CUDA_CHECK(cudaGetLastError());
    }
    if (small_blocks > 0) {
        wa_small_kernel<<<small_blocks, GPU_THREADS, 0, s>>>(g->num_small, g->warp_base,
                                                             g->small_deg, g->px, g->py,
                                                             g->slot_cell, g->num_cells,
                                                             inv_gamma, dgrad, g->partial);
        CUDA_CHECK(cudaGetLastError());
    }
    if (g->num_big > 0) {
        wa_big_kernel<<<g->num_big, GPU_THREADS, 0, s>>>(g->big_start, g->big_deg, g->px, g->py,
                                                         g->slot_cell, g->num_cells, inv_gamma,
                                                         dgrad, g->partial + small_blocks);
        CUDA_CHECK(cudaGetLastError());
    }
    CUDA_CHECK(cudaEventRecord(g->ev[EV_WL_END], s));
}

// trae el gradiente y las sumas parciales; solo espera a s_wl, la densidad
// puede seguir corriendo en la GPU mientras tanto
double wirelength_wait(GpuPlacer *g, float *grad)
{
    size_t bytes = 2 * (size_t)g->num_cells * sizeof(float);
    cudaStream_t s = g->s_wl;
    if (grad && g->wl_grad)
        CUDA_CHECK(cudaMemcpyAsync(grad, g->grad, bytes, cudaMemcpyDeviceToHost, s));
    if (g->wl_parts > 0)
        CUDA_CHECK(cudaMemcpyAsync(g->h_partial, g->partial, g->wl_parts * sizeof(float),
                                   cudaMemcpyDeviceToHost, s));
    CUDA_CHECK(cudaEventRecord(g->ev[EV_WL_D2H], s));
    CUDA_CHECK(cudaEventSynchronize(g->ev[EV_WL_D2H]));

    gpu_count_upload(g);
    prof_add(PROF_WL_KERNEL, gpu_elapsed(g->ev[EV_UP1], g->ev[EV_WL_END]));
    prof_add(PROF_D2H, gpu_elapsed(g->ev[EV_WL_END], g->ev[EV_WL_D2H]));

    if (grad && !g->wl_grad)
        memset(grad, 0, bytes);
    double total = 0.0;
    for (int b = 0; b < g->wl_parts; b++)
        total += g->h_partial[b];
    return total;
}
