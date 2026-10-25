#ifndef GPU_INTERNAL_H
#define GPU_INTERNAL_H

// Lo que comparten los .cu de la version CUDA. Solo se incluye desde ellos.
#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>
#include <cufft.h>
#include "gpu.h"
#include "prof.h"

#define GPU_THREADS 256

// en este prototipo cualquier error de CUDA corta la corrida con un mensaje
#define CUDA_CHECK(call) do { \
    cudaError_t err_ = (call); \
    if (err_ != cudaSuccess) { \
        fprintf(stderr, "error de CUDA en %s:%d: %s\n", __FILE__, __LINE__, \
                cudaGetErrorString(err_)); \
        exit(1); \
    } \
} while (0)

#define CUFFT_CHECK(call) do { \
    cufftResult err_ = (call); \
    if (err_ != CUFFT_SUCCESS) { \
        fprintf(stderr, "error de cuFFT en %s:%d: codigo %d\n", __FILE__, __LINE__, \
                (int)err_); \
        exit(1); \
    } \
} while (0)

// geometria de la grilla; se pasa por valor a los kernels
typedef struct {
    int nx, ny;
    float x0, y0;
    float bin_w, bin_h;
} GridGeom;

// DCT-II 2D de n*n con cuFFT (Makhoul: reordenar, FFT real y un giro por
// frecuencia), primero por filas y despues por columnas
typedef struct {
    int n;
    cufftHandle r2c, c2r;     // n transformadas de largo n en cada llamada
    float *work;              // n*n, una linea reordenada por fila
    cufftComplex *spec;       // n*(n/2+1), medio espectro de cada linea
    float2 *twiddle;          // (cos, sin) de pi*k/(2n), k = 0..n-1
} DctGpu;

void dct_gpu_init(DctGpu *d, int n);
void dct_gpu_free(DctGpu *d);
// data esta en la GPU (n*n, fila iy*n+ix) y se transforma en su lugar
void dct_gpu_forward(DctGpu *d, float *data);
void dct_gpu_inverse(DctGpu *d, float *data);

// eventos para medir cada parte de una llamada sin sincronizar entre medio
#define GPU_EVENTS 8

struct GpuPlacer {
    int num_cells, num_nets, num_pins;

    // netlist: se sube una sola vez
    Net *nets;
    Pin *pins;
    float *cell_w, *cell_h;
    unsigned char *cell_fixed;

    // por iteracion: posiciones (2N) y un gradiente (2N) que usan primero
    // wirelength y despues densidad
    float *v;
    float *grad;

    // sumas parciales por bloque, que el host termina en double
    float *partial;
    float *h_partial;

    // grilla de densidad
    GridGeom geo;
    float movable_area;
    float *density, *phi, *ex, *ey;
    float *inv_w2;            // 1/(wx^2+wy^2) por bin, 0 en el DC
    DctGpu dct;

    cudaEvent_t ev[GPU_EVENTS];
};

void density_gpu_init(GpuPlacer *g, const Grid *grid);
void density_gpu_free(GpuPlacer *g);

// segundos entre dos eventos que ya se completaron
double gpu_elapsed(cudaEvent_t a, cudaEvent_t b);

#endif
