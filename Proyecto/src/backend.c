#include <stdlib.h>
#include "backend.h"
#include "wirelength.h"
#include "density.h"
#ifdef USE_CUDA
#include "gpu.h"
#endif

// este es el unico archivo que cambia entre la version CPU y la CUDA: el
// Makefile lo compila con USE_CUDA para build/placer_gpu
struct Backend {
    const Netlist *nl;
    Grid *grid;
    const float *v;      // punto de la evaluacion en curso
    float gamma;
#ifdef USE_CUDA
    GpuPlacer *gpu;
#endif
};

Backend *backend_create(const Netlist *nl, Grid *grid)
{
    Backend *b = malloc(sizeof *b);
    if (!b)
        return NULL;
    b->nl = nl;
    b->grid = grid;
#ifdef USE_CUDA
    b->gpu = gpu_create(nl, grid);
    if (!b->gpu) {
        free(b);
        return NULL;
    }
#endif
    return b;
}

void backend_destroy(Backend *b)
{
    if (!b)
        return;
#ifdef USE_CUDA
    gpu_destroy(b->gpu);
#endif
    free(b);
}

void backend_eval_start(Backend *b, const float *v, float gamma)
{
    b->v = v;
    b->gamma = gamma;
#ifdef USE_CUDA
    gpu_eval_start(b->gpu, v, gamma);
#endif
}

double backend_wait_wirelength(Backend *b, float *grad)
{
#ifdef USE_CUDA
    return gpu_wait_wirelength(b->gpu, grad);
#else
    return compute_wirelength(b->nl, b->v, b->gamma, grad);
#endif
}

double backend_wait_density(Backend *b, float *grad, float *overflow)
{
#ifdef USE_CUDA
    return gpu_wait_density(b->gpu, grad, overflow);
#else
    return compute_density(b->nl, b->grid, b->v, grad, overflow);
#endif
}
