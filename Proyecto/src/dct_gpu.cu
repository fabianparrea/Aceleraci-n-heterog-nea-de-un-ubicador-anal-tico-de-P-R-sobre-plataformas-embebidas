#include <math.h>
#include "gpu_internal.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// DCT-II de largo n con una sola FFT real de largo n (Makhoul, 1980):
//   1. v[i] = x[makhoul_src(i)]: primero los pares en orden, despues los
//      impares al reves
//   2. V = FFT(v)
//   3. X[k] = Re(V[k] * e^(-i*pi*k/(2n)))
// La inversa recorre los mismos pasos al reves. Da la misma DCT sin escalar de
// dct.c (y su inversa exacta), pero en O(n log n) por linea en vez de O(n^2).

static __device__ int makhoul_src(int i, int n)
{
    return i < n / 2 ? 2 * i : 2 * (n - 1 - i) + 1;
}

// Cada pasada transforma n lineas: las filas de data (cols = 0) o sus columnas
// (cols = 1). work y spec guardan siempre una linea por fila, asi cuFFT trabaja
// con lotes contiguos en las dos pasadas; la columna se transpone al leer data
// y otra vez al escribirla.
static __device__ int line_at(int line, int pos, int n, int cols)
{
    return cols ? pos * n + line : line * n + pos;
}

__global__ void dct_reorder_kernel(const float *data, float *work, int n, int cols)
{
    int line = blockIdx.y;
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n)
        work[line * n + i] = data[line_at(line, makhoul_src(i, n), n, cols)];
}

// cuFFT (R2C) solo deja V[0..n/2]; el resto sale de la simetria de una entrada
// real: V[k] = conj(V[n-k])
__global__ void dct_post_kernel(const cufftComplex *spec, const float2 *tw, float *data,
                                int n, int cols)
{
    int line = blockIdx.y;
    int k = blockIdx.x * blockDim.x + threadIdx.x;
    if (k >= n)
        return;
    const cufftComplex *row = spec + (size_t)line * (n / 2 + 1);
    float re, im;
    if (k <= n / 2) {
        re = row[k].x;
        im = row[k].y;
    } else {
        re = row[n - k].x;
        im = -row[n - k].y;
    }
    // Re((re + i*im) * (cos - i*sin))
    data[line_at(line, k, n, cols)] = re * tw[k].x + im * tw[k].y;
}

// inversa: V[k] = e^(i*pi*k/(2n)) * (X[k] - i*X[n-k]), con X[n] = 0, es justo
// la FFT del arreglo reordenado; para C2R alcanza con la mitad k = 0..n/2
__global__ void dct_pre_kernel(const float *data, const float2 *tw, cufftComplex *spec,
                               int n, int cols)
{
    int line = blockIdx.y;
    int k = blockIdx.x * blockDim.x + threadIdx.x;
    if (k > n / 2)
        return;
    float a = data[line_at(line, k, n, cols)];
    float b = k > 0 ? data[line_at(line, n - k, n, cols)] : 0.0f;
    float c = tw[k].x, s = tw[k].y;
    cufftComplex out;
    out.x = c * a + s * b;
    out.y = s * a - c * b;
    spec[(size_t)line * (n / 2 + 1) + k] = out;
}

// deshace el reordenamiento; C2R no normaliza, por eso se divide por n
__global__ void dct_unreorder_kernel(const float *work, float *data, int n, int cols)
{
    int line = blockIdx.y;
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n)
        data[line_at(line, makhoul_src(i, n), n, cols)] = work[line * n + i] / n;
}

void dct_gpu_init(DctGpu *d, int n)
{
    d->n = n;
    CUFFT_CHECK(cufftPlan1d(&d->r2c, n, CUFFT_R2C, n));
    CUFFT_CHECK(cufftPlan1d(&d->c2r, n, CUFFT_C2R, n));
    CUDA_CHECK(cudaMalloc(&d->work, (size_t)n * n * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&d->spec, (size_t)n * (n / 2 + 1) * sizeof(cufftComplex)));

    // los giros se calculan en double y se guardan en float
    float2 *tw = (float2 *)malloc(n * sizeof(float2));
    for (int k = 0; k < n; k++) {
        double angle = M_PI * k / (2.0 * n);
        tw[k].x = (float)cos(angle);
        tw[k].y = (float)sin(angle);
    }
    CUDA_CHECK(cudaMalloc(&d->twiddle, n * sizeof(float2)));
    CUDA_CHECK(cudaMemcpy(d->twiddle, tw, n * sizeof(float2), cudaMemcpyHostToDevice));
    free(tw);
}

void dct_gpu_free(DctGpu *d)
{
    cufftDestroy(d->r2c);
    cufftDestroy(d->c2r);
    cudaFree(d->work);
    cudaFree(d->spec);
    cudaFree(d->twiddle);
}

void dct_gpu_forward(DctGpu *d, float *data)
{
    int n = d->n;
    dim3 blocks((n + GPU_THREADS - 1) / GPU_THREADS, n);
    // primero filas y despues columnas, en el mismo orden que dct.c
    for (int cols = 0; cols < 2; cols++) {
        dct_reorder_kernel<<<blocks, GPU_THREADS>>>(data, d->work, n, cols);
        CUDA_CHECK(cudaGetLastError());
        CUFFT_CHECK(cufftExecR2C(d->r2c, d->work, d->spec));
        dct_post_kernel<<<blocks, GPU_THREADS>>>(d->spec, d->twiddle, data, n, cols);
        CUDA_CHECK(cudaGetLastError());
    }
}

void dct_gpu_inverse(DctGpu *d, float *data)
{
    int n = d->n;
    dim3 blocks((n + GPU_THREADS - 1) / GPU_THREADS, n);
    for (int cols = 0; cols < 2; cols++) {
        dct_pre_kernel<<<blocks, GPU_THREADS>>>(data, d->twiddle, d->spec, n, cols);
        CUDA_CHECK(cudaGetLastError());
        CUFFT_CHECK(cufftExecC2R(d->c2r, d->spec, d->work));
        dct_unreorder_kernel<<<blocks, GPU_THREADS>>>(d->work, data, n, cols);
        CUDA_CHECK(cudaGetLastError());
    }
}

int gpu_dct2d(float *data, int n, int inverse)
{
    int count = 0;
    if (cudaGetDeviceCount(&count) != cudaSuccess || count == 0)
        return -1;
    DctGpu d;
    dct_gpu_init(&d, n);
    size_t bytes = (size_t)n * n * sizeof(float);
    float *dev = NULL;
    CUDA_CHECK(cudaMalloc(&dev, bytes));
    CUDA_CHECK(cudaMemcpy(dev, data, bytes, cudaMemcpyHostToDevice));
    if (inverse)
        dct_gpu_inverse(&d, dev);
    else
        dct_gpu_forward(&d, dev);
    CUDA_CHECK(cudaMemcpy(data, dev, bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaFree(dev));
    dct_gpu_free(&d);
    return 0;
}
