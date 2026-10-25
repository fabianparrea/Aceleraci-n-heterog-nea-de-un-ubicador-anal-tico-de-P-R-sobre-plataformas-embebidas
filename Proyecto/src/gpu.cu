#include "gpu_internal.h"

double gpu_elapsed(cudaEvent_t a, cudaEvent_t b)
{
    float ms = 0.0f;
    CUDA_CHECK(cudaEventElapsedTime(&ms, a, b));
    return ms * 1e-3;
}

// reserva en la GPU y copia desde el host
static void *upload(const void *src, size_t bytes)
{
    void *dst = NULL;
    CUDA_CHECK(cudaMalloc(&dst, bytes));
    CUDA_CHECK(cudaMemcpy(dst, src, bytes, cudaMemcpyHostToDevice));
    return dst;
}

GpuPlacer *gpu_create(const Netlist *nl, const Grid *grid)
{
    double t0 = prof_now();
    int count = 0;
    if (cudaGetDeviceCount(&count) != cudaSuccess || count == 0) {
        fprintf(stderr, "no hay ninguna GPU CUDA disponible\n");
        return NULL;
    }
    // la DCT de la GPU usa una sola FFT para filas y columnas: necesita la
    // grilla cuadrada, que es como la arma grid_init
    if (grid->nx != grid->ny) {
        fprintf(stderr, "la version GPU necesita una grilla cuadrada (es %dx%d)\n",
                grid->nx, grid->ny);
        return NULL;
    }

    GpuPlacer *g = (GpuPlacer *)calloc(1, sizeof *g);
    int n = nl->num_cells;
    g->num_cells = n;
    g->num_nets = nl->num_nets;
    g->num_pins = nl->num_pins;

    // Net y Pin son structs planos y se suben tal cual; de Cell solo hace
    // falta el tamano y si es fija (el nombre y lo demas no se usan aca)
    g->nets = (Net *)upload(nl->nets, (size_t)nl->num_nets * sizeof(Net));
    g->pins = (Pin *)upload(nl->pins, (size_t)nl->num_pins * sizeof(Pin));

    float *w = (float *)malloc(n * sizeof(float));
    float *h = (float *)malloc(n * sizeof(float));
    unsigned char *fixed = (unsigned char *)malloc(n);
    for (int i = 0; i < n; i++) {
        w[i] = nl->cells[i].width;
        h[i] = nl->cells[i].height;
        fixed[i] = nl->cells[i].fixed != 0;
    }
    g->cell_w = (float *)upload(w, n * sizeof(float));
    g->cell_h = (float *)upload(h, n * sizeof(float));
    g->cell_fixed = (unsigned char *)upload(fixed, n);
    free(w);
    free(h);
    free(fixed);

    CUDA_CHECK(cudaMalloc(&g->v, 2 * (size_t)n * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&g->grad, 2 * (size_t)n * sizeof(float)));

    // un parcial por bloque: wirelength usa uno por bloque de redes, densidad
    // dos por bloque de bins (energia y overflow)
    int net_blocks = (nl->num_nets + GPU_THREADS - 1) / GPU_THREADS;
    int bin_blocks = (grid->nx * grid->ny + GPU_THREADS - 1) / GPU_THREADS;
    int num_partial = net_blocks > 2 * bin_blocks ? net_blocks : 2 * bin_blocks;
    CUDA_CHECK(cudaMalloc(&g->partial, num_partial * sizeof(float)));
    g->h_partial = (float *)malloc(num_partial * sizeof(float));

    density_gpu_init(g, grid);
    for (int e = 0; e < GPU_EVENTS; e++)
        CUDA_CHECK(cudaEventCreate(&g->ev[e]));

    cudaDeviceProp prop;
    CUDA_CHECK(cudaGetDeviceProperties(&prop, 0));
    printf("GPU: %s (sm_%d%d, %d SM), lista en %.2f s\n", prop.name, prop.major,
           prop.minor, prop.multiProcessorCount, prof_now() - t0);
    return g;
}

void gpu_destroy(GpuPlacer *g)
{
    if (!g)
        return;
    density_gpu_free(g);
    for (int e = 0; e < GPU_EVENTS; e++)
        cudaEventDestroy(g->ev[e]);
    cudaFree(g->nets);
    cudaFree(g->pins);
    cudaFree(g->cell_w);
    cudaFree(g->cell_h);
    cudaFree(g->cell_fixed);
    cudaFree(g->v);
    cudaFree(g->grad);
    cudaFree(g->partial);
    free(g->h_partial);
    free(g);
}
