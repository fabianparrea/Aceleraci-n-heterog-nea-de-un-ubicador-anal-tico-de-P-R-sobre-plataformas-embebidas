#include <math.h>
#include "gpu_internal.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Las mismas cuentas de density.c, con un hilo por celda o por bin.

static __device__ float overlap_1d(float a0, float a1, float b0, float b1)
{
    float lo = a0 > b0 ? a0 : b0;
    float hi = a1 < b1 ? a1 : b1;
    return hi > lo ? hi - lo : 0.0f;
}

static __device__ int clamp_index(int i, int n)
{
    return i < 0 ? 0 : (i > n ? n : i);
}

// rango de bins que toca una celda, recortado a la grilla (como cell_bin_range)
static __device__ void cell_bin_range(GridGeom gg, float cx, float cy, float w, float h,
                                      float *x0, float *x1, float *y0, float *y1,
                                      int *ix0, int *ix1, int *iy0, int *iy1)
{
    *x0 = cx - w / 2;
    *x1 = cx + w / 2;
    *y0 = cy - h / 2;
    *y1 = cy + h / 2;
    *ix0 = clamp_index((int)floorf((*x0 - gg.x0) / gg.bin_w), gg.nx);
    *ix1 = clamp_index((int)ceilf((*x1 - gg.x0) / gg.bin_w), gg.nx);
    *iy0 = clamp_index((int)floorf((*y0 - gg.y0) / gg.bin_h), gg.ny);
    *iy1 = clamp_index((int)ceilf((*y1 - gg.y0) / gg.bin_h), gg.ny);
}

// un hilo por celda (las fijas tambien cuentan, como en la CPU); varias celdas
// pueden caer en el mismo bin, por eso atomicAdd
__global__ void splat_kernel(int n, const float *v, const float *cell_w, const float *cell_h,
                             GridGeom gg, float *density)
{
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= n)
        return;
    float x0, x1, y0, y1;
    int ix0, ix1, iy0, iy1;
    cell_bin_range(gg, v[i], v[n + i], cell_w[i], cell_h[i],
                   &x0, &x1, &y0, &y1, &ix0, &ix1, &iy0, &iy1);
    for (int iy = iy0; iy < iy1; iy++) {
        float by0 = gg.y0 + iy * gg.bin_h;
        float oy = overlap_1d(y0, y1, by0, by0 + gg.bin_h);
        if (oy <= 0)
            continue;
        for (int ix = ix0; ix < ix1; ix++) {
            float bx0 = gg.x0 + ix * gg.bin_w;
            float ox = overlap_1d(x0, x1, bx0, bx0 + gg.bin_w);
            if (ox <= 0)
                continue;
            atomicAdd(&density[iy * gg.nx + ix], ox * oy);
        }
    }
}

// Poisson en frecuencia: dividir por (wx^2+wy^2), aca multiplicar por la tabla
__global__ void scale_kernel(int nb, const float *inv_w2, float *phi)
{
    int b = blockIdx.x * blockDim.x + threadIdx.x;
    if (b < nb)
        phi[b] *= inv_w2[b];
}

// campo = -gradiente del potencial, por diferencias centradas (como compute_field)
__global__ void field_kernel(GridGeom gg, const float *phi, float *ex, float *ey)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= gg.nx * gg.ny)
        return;
    int ix = idx % gg.nx, iy = idx / gg.nx;
    int il = ix > 0 ? ix - 1 : ix;
    int ir = ix < gg.nx - 1 ? ix + 1 : ix;
    int ib = iy > 0 ? iy - 1 : iy;
    int it = iy < gg.ny - 1 ? iy + 1 : iy;

    float dx = (ir - il) * gg.bin_w;
    float dy = (it - ib) * gg.bin_h;
    ex[idx] = -(phi[iy * gg.nx + ir] - phi[iy * gg.nx + il]) / dx;
    ey[idx] = -(phi[it * gg.nx + ix] - phi[ib * gg.nx + ix]) / dy;
}

// energia (suma de density*phi) y exceso sobre la capacidad de cada bin,
// sumados por bloque; el host termina las dos sumas en double
__global__ void sums_kernel(int nb, const float *density, const float *phi, float bin_cap,
                            float *partial_energy, float *partial_over)
{
    __shared__ float se[GPU_THREADS], so[GPU_THREADS];
    int b = blockIdx.x * blockDim.x + threadIdx.x;
    float e = 0.0f, o = 0.0f;
    if (b < nb) {
        e = density[b] * phi[b];
        float excess = density[b] - bin_cap;
        o = excess > 0 ? excess : 0.0f;
    }
    se[threadIdx.x] = e;
    so[threadIdx.x] = o;
    __syncthreads();
    for (unsigned s = blockDim.x / 2; s > 0; s >>= 1) {
        if (threadIdx.x < s) {
            se[threadIdx.x] += se[threadIdx.x + s];
            so[threadIdx.x] += so[threadIdx.x + s];
        }
        __syncthreads();
    }
    if (threadIdx.x == 0) {
        partial_energy[blockIdx.x] = se[0];
        partial_over[blockIdx.x] = so[0];
    }
}

// un hilo por celda: el campo promedio de los bins que toca, pesado por cuanto
// cubre de cada uno. Cada hilo escribe solo su par grad[i]/grad[n+i].
__global__ void density_grad_kernel(int n, const float *v, const float *cell_w,
                                    const float *cell_h, const unsigned char *fixed,
                                    GridGeom gg, const float *ex, const float *ey,
                                    float *grad)
{
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= n)
        return;
    float gx = 0.0f, gy = 0.0f;
    if (!fixed[i]) {
        float x0, x1, y0, y1;
        int ix0, ix1, iy0, iy1;
        cell_bin_range(gg, v[i], v[n + i], cell_w[i], cell_h[i],
                       &x0, &x1, &y0, &y1, &ix0, &ix1, &iy0, &iy1);
        for (int iy = iy0; iy < iy1; iy++) {
            float by0 = gg.y0 + iy * gg.bin_h;
            float oy = overlap_1d(y0, y1, by0, by0 + gg.bin_h);
            if (oy <= 0)
                continue;
            for (int ix = ix0; ix < ix1; ix++) {
                float bx0 = gg.x0 + ix * gg.bin_w;
                float ox = overlap_1d(x0, x1, bx0, bx0 + gg.bin_w);
                if (ox <= 0)
                    continue;
                int idx = iy * gg.nx + ix;
                float area = ox * oy;
                gx -= area * ex[idx];
                gy -= area * ey[idx];
            }
        }
    }
    grad[i] = gx;
    grad[n + i] = gy;
}

void density_gpu_init(GpuPlacer *g, const Grid *grid)
{
    GridGeom gg = {grid->nx, grid->ny, grid->x0, grid->y0, grid->bin_w, grid->bin_h};
    g->geo = gg;
    g->movable_area = grid->movable_area;

    int nx = grid->nx, ny = grid->ny;
    size_t bytes = (size_t)nx * ny * sizeof(float);
    CUDA_CHECK(cudaMalloc(&g->density, bytes));
    CUDA_CHECK(cudaMalloc(&g->phi, bytes));
    CUDA_CHECK(cudaMalloc(&g->ex, bytes));
    CUDA_CHECK(cudaMalloc(&g->ey, bytes));

    // 1/(wx^2+wy^2) por bin, en double como en solve_poisson; el DC queda en
    // cero porque el nivel base del potencial no importa
    float *table = (float *)malloc(bytes);
    for (int iy = 0; iy < ny; iy++) {
        double wy = M_PI * iy / ny;
        for (int ix = 0; ix < nx; ix++) {
            double wx = M_PI * ix / nx;
            table[iy * nx + ix] = (ix == 0 && iy == 0) ? 0.0f : (float)(1.0 / (wx * wx + wy * wy));
        }
    }
    CUDA_CHECK(cudaMalloc(&g->inv_w2, bytes));
    CUDA_CHECK(cudaMemcpy(g->inv_w2, table, bytes, cudaMemcpyHostToDevice));
    free(table);

    dct_gpu_init(&g->dct, nx);
}

void density_gpu_free(GpuPlacer *g)
{
    dct_gpu_free(&g->dct);
    cudaFree(g->density);
    cudaFree(g->phi);
    cudaFree(g->ex);
    cudaFree(g->ey);
    cudaFree(g->inv_w2);
}

double gpu_density(GpuPlacer *g, const float *v, float *grad, float *overflow)
{
    int n = g->num_cells;
    int nb = g->geo.nx * g->geo.ny;
    int cell_blocks = (n + GPU_THREADS - 1) / GPU_THREADS;
    int bin_blocks = (nb + GPU_THREADS - 1) / GPU_THREADS;
    size_t bytes = 2 * (size_t)n * sizeof(float);
    cudaEvent_t *ev = g->ev;

    CUDA_CHECK(cudaEventRecord(ev[0]));
    CUDA_CHECK(cudaMemcpy(g->v, v, bytes, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaEventRecord(ev[1]));

    CUDA_CHECK(cudaMemset(g->density, 0, nb * sizeof(float)));
    splat_kernel<<<cell_blocks, GPU_THREADS>>>(n, g->v, g->cell_w, g->cell_h, g->geo,
                                               g->density);
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaEventRecord(ev[2]));

    CUDA_CHECK(cudaMemcpy(g->phi, g->density, nb * sizeof(float), cudaMemcpyDeviceToDevice));
    dct_gpu_forward(&g->dct, g->phi);
    scale_kernel<<<bin_blocks, GPU_THREADS>>>(nb, g->inv_w2, g->phi);
    CUDA_CHECK(cudaGetLastError());
    dct_gpu_inverse(&g->dct, g->phi);
    CUDA_CHECK(cudaEventRecord(ev[3]));

    field_kernel<<<bin_blocks, GPU_THREADS>>>(g->geo, g->phi, g->ex, g->ey);
    CUDA_CHECK(cudaGetLastError());
    float bin_cap = g->geo.bin_w * g->geo.bin_h;   // target de ocupacion = 100%
    sums_kernel<<<bin_blocks, GPU_THREADS>>>(nb, g->density, g->phi, bin_cap,
                                             g->partial, g->partial + bin_blocks);
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaEventRecord(ev[4]));

    if (grad) {
        density_grad_kernel<<<cell_blocks, GPU_THREADS>>>(n, g->v, g->cell_w, g->cell_h,
                                                          g->cell_fixed, g->geo,
                                                          g->ex, g->ey, g->grad);
        CUDA_CHECK(cudaGetLastError());
    }
    CUDA_CHECK(cudaEventRecord(ev[5]));

    CUDA_CHECK(cudaMemcpy(g->h_partial, g->partial, 2 * bin_blocks * sizeof(float),
                          cudaMemcpyDeviceToHost));
    if (grad)
        CUDA_CHECK(cudaMemcpy(grad, g->grad, bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaEventRecord(ev[6]));
    CUDA_CHECK(cudaEventSynchronize(ev[6]));

    prof_add(PROF_H2D, gpu_elapsed(ev[0], ev[1]));
    prof_add(PROF_SPLAT, gpu_elapsed(ev[1], ev[2]));
    prof_add(PROF_POISSON, gpu_elapsed(ev[2], ev[3]));
    prof_add(PROF_FIELD, gpu_elapsed(ev[3], ev[4]));
    prof_add(PROF_DENSITY_GRAD, gpu_elapsed(ev[4], ev[5]));
    prof_add(PROF_D2H, gpu_elapsed(ev[5], ev[6]));

    double energy = 0.0, over = 0.0;
    for (int b = 0; b < bin_blocks; b++) {
        energy += g->h_partial[b];
        over += g->h_partial[bin_blocks + b];
    }
    if (overflow)
        *overflow = g->movable_area > 0 ? (float)(over / g->movable_area) : 0.0f;
    return 0.5 * energy;
}
