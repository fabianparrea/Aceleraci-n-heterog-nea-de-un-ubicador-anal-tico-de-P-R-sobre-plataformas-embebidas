#include <float.h>
#include <string.h>
#include "gpu_internal.h"

// Un hilo por red, como en el diseno del paper: cada red se calcula por su
// cuenta y lo unico compartido es el gradiente de una celda que esta en varias
// redes, que se acumula con atomicAdd (el mismo atomico de OpenMP en la CPU).
// Hace las dos dimensiones en la misma pasada para leer los pines una sola vez.
__global__ void wa_kernel(const Net *nets, const Pin *pins, int num_nets, int n,
                          const float *v, float inv_gamma, float *grad, float *partial)
{
    __shared__ float block_sum[GPU_THREADS];
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    float wa = 0.0f;

    if (i < num_nets && nets[i].degree >= 2) {
        const Pin *p = pins + nets[i].pin_start;
        int deg = nets[i].degree;

        float xlo = FLT_MAX, xhi = -FLT_MAX, ylo = FLT_MAX, yhi = -FLT_MAX;
        for (int k = 0; k < deg; k++) {
            float x = v[p[k].cell] + p[k].dx;
            float y = v[n + p[k].cell] + p[k].dy;
            xlo = fminf(xlo, x);
            xhi = fmaxf(xhi, x);
            ylo = fminf(ylo, y);
            yhi = fmaxf(yhi, y);
        }

        // la CPU lleva las sumas ponderadas en double; aca en float se miden
        // desde el extremo que domina cada lado (el lado + desde el maximo, el
        // - desde el minimo), para que un pin muy lejos del resto (una celda que
        // un mal paso mando afuera) no deje la resta del gradiente sin precision.
        // Es la misma cuenta, solo cambia el origen.
        float sxp = 0, wxp = 0, sxm = 0, wxm = 0;
        float syp = 0, wyp = 0, sym = 0, wym = 0;
        for (int k = 0; k < deg; k++) {
            float x = v[p[k].cell] + p[k].dx;
            float y = v[n + p[k].cell] + p[k].dy;
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
        // promedio ponderado hacia el maximo (medido desde el maximo, <= 0) y
        // hacia el minimo (medido desde el minimo, >= 0)
        float mxp = wxp / sxp, mxm = wxm / sxm;
        float myp = wyp / syp, mym = wym / sym;
        wa = ((xhi - xlo) + mxp - mxm) + ((yhi - ylo) + myp - mym);

        if (grad) {
            for (int k = 0; k < deg; k++) {
                float x = v[p[k].cell] + p[k].dx;
                float y = v[n + p[k].cell] + p[k].dy;
                float axp = expf((x - xhi) * inv_gamma), axm = expf((xlo - x) * inv_gamma);
                float ayp = expf((y - yhi) * inv_gamma), aym = expf((ylo - y) * inv_gamma);
                float gx = axp / sxp * (1 + (x - xhi - mxp) * inv_gamma)
                         - axm / sxm * (1 - (x - xlo - mxm) * inv_gamma);
                float gy = ayp / syp * (1 + (y - yhi - myp) * inv_gamma)
                         - aym / sym * (1 - (y - ylo - mym) * inv_gamma);
                atomicAdd(&grad[p[k].cell], gx);
                atomicAdd(&grad[n + p[k].cell], gy);
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

double gpu_wirelength(GpuPlacer *g, const float *v, float gamma, float *grad)
{
    size_t bytes = 2 * (size_t)g->num_cells * sizeof(float);
    int blocks = (g->num_nets + GPU_THREADS - 1) / GPU_THREADS;
    if (blocks == 0) {
        if (grad)
            memset(grad, 0, bytes);
        return 0.0;
    }
    cudaEvent_t *ev = g->ev;

    CUDA_CHECK(cudaEventRecord(ev[0]));
    CUDA_CHECK(cudaMemcpy(g->v, v, bytes, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaEventRecord(ev[1]));
    if (grad)
        CUDA_CHECK(cudaMemset(g->grad, 0, bytes));
    wa_kernel<<<blocks, GPU_THREADS>>>(g->nets, g->pins, g->num_nets, g->num_cells, g->v,
                                       1.0f / gamma, grad ? g->grad : NULL, g->partial);
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaEventRecord(ev[2]));
    if (grad)
        CUDA_CHECK(cudaMemcpy(grad, g->grad, bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(g->h_partial, g->partial, blocks * sizeof(float),
                          cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaEventRecord(ev[3]));
    CUDA_CHECK(cudaEventSynchronize(ev[3]));

    prof_add(PROF_H2D, gpu_elapsed(ev[0], ev[1]));
    prof_add(PROF_WL_KERNEL, gpu_elapsed(ev[1], ev[2]));
    prof_add(PROF_D2H, gpu_elapsed(ev[2], ev[3]));

    double total = 0.0;
    for (int b = 0; b < blocks; b++)
        total += g->h_partial[b];
    return total;
}
