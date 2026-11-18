#include "gpu_internal.h"

double gpu_elapsed(cudaEvent_t a, cudaEvent_t b)
{
    float ms = 0.0f;
    CUDA_CHECK(cudaEventElapsedTime(&ms, a, b));
    return ms * 1e-3;
}

void *gpu_upload(const void *src, size_t bytes)
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

    // en CUDA el numero mas chico es la prioridad mas alta
    int low = 0, high = 0;
    CUDA_CHECK(cudaDeviceGetStreamPriorityRange(&low, &high));
    CUDA_CHECK(cudaStreamCreateWithPriority(&g->s_wl, cudaStreamDefault, high));
    CUDA_CHECK(cudaStreamCreateWithPriority(&g->s_d, cudaStreamDefault, low));

    // de Cell solo hace falta el tamano y si es fija (el nombre y lo demas no
    // se usan aca); las redes y los pines los arma wirelength_gpu_init
    wirelength_gpu_init(g, nl);

    float *w = (float *)malloc(n * sizeof(float));
    float *h = (float *)malloc(n * sizeof(float));
    unsigned char *fixed = (unsigned char *)malloc(n);
    for (int i = 0; i < n; i++) {
        w[i] = nl->cells[i].width;
        h[i] = nl->cells[i].height;
        fixed[i] = nl->cells[i].fixed != 0;
    }
    g->cell_w = (float *)gpu_upload(w, n * sizeof(float));
    g->cell_h = (float *)gpu_upload(h, n * sizeof(float));
    g->cell_fixed = (unsigned char *)gpu_upload(fixed, n);
    free(w);
    free(h);
    free(fixed);

    CUDA_CHECK(cudaMalloc(&g->v, 2 * (size_t)n * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&g->grad, 2 * (size_t)n * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&g->grad_d, 2 * (size_t)n * sizeof(float)));

    g->wl_parts = wirelength_gpu_partials(g);
    g->bin_blocks = (grid->nx * grid->ny + GPU_THREADS - 1) / GPU_THREADS;
    int num_partial = g->wl_parts + 2 * g->bin_blocks;
    CUDA_CHECK(cudaMalloc(&g->partial, num_partial * sizeof(float)));
    g->h_partial = (float *)malloc(num_partial * sizeof(float));

    density_gpu_init(g, grid);
    for (int e = 0; e < GPU_EVENTS; e++)
        CUDA_CHECK(cudaEventCreate(&g->ev[e]));
    // que todas las subidas de arriba hayan terminado antes de la primera
    // iteracion, que ya corre en los otros streams
    CUDA_CHECK(cudaDeviceSynchronize());

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
    cudaDeviceSynchronize();
    density_gpu_free(g);
    for (int e = 0; e < GPU_EVENTS; e++)
        cudaEventDestroy(g->ev[e]);
    wirelength_gpu_free(g);
    cudaFree(g->cell_w);
    cudaFree(g->cell_h);
    cudaFree(g->cell_fixed);
    cudaFree(g->v);
    cudaFree(g->grad);
    cudaFree(g->grad_d);
    cudaFree(g->partial);
    free(g->h_partial);
    cudaStreamDestroy(g->s_wl);
    cudaStreamDestroy(g->s_d);
    free(g);
}

// v se sube por el stream de wirelength; densidad espera a EV_UP1 antes de
// empezar, asi los dos calculos usan la misma copia
void gpu_upload_v(GpuPlacer *g, const float *v)
{
    size_t bytes = 2 * (size_t)g->num_cells * sizeof(float);
    CUDA_CHECK(cudaEventRecord(g->ev[EV_UP0], g->s_wl));
    CUDA_CHECK(cudaMemcpyAsync(g->v, v, bytes, cudaMemcpyHostToDevice, g->s_wl));
    CUDA_CHECK(cudaEventRecord(g->ev[EV_UP1], g->s_wl));
    g->h2d_pending = 1;
}

void gpu_count_upload(GpuPlacer *g)
{
    if (g->h2d_pending) {
        prof_add(PROF_H2D, gpu_elapsed(g->ev[EV_UP0], g->ev[EV_UP1]));
        g->h2d_pending = 0;
    }
}

void gpu_eval_start(GpuPlacer *g, const float *v, float gamma)
{
    gpu_upload_v(g, v);
    wirelength_enqueue(g, gamma, 1);
    density_enqueue(g, 1);
}

double gpu_wait_wirelength(GpuPlacer *g, float *grad)
{
    return wirelength_wait(g, grad);
}

double gpu_wait_density(GpuPlacer *g, float *grad, float *overflow)
{
    return density_wait(g, grad, overflow);
}

double gpu_wirelength(GpuPlacer *g, const float *v, float gamma, float *grad)
{
    gpu_upload_v(g, v);
    wirelength_enqueue(g, gamma, grad != NULL);
    return wirelength_wait(g, grad);
}

double gpu_density(GpuPlacer *g, const float *v, float *grad, float *overflow)
{
    gpu_upload_v(g, v);
    density_enqueue(g, grad != NULL);
    return density_wait(g, grad, overflow);
}
