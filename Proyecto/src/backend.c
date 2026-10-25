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

double backend_wirelength(Backend *b, const float *v, float gamma, float *grad)
{
#ifdef USE_CUDA
    return gpu_wirelength(b->gpu, v, gamma, grad);
#else
    return compute_wirelength(b->nl, v, gamma, grad);
#endif
}

double backend_density(Backend *b, const float *v, float *grad, float *overflow)
{
#ifdef USE_CUDA
    return gpu_density(b->gpu, v, grad, overflow);
#else
    return compute_density(b->nl, b->grid, v, grad, overflow);
#endif
}
