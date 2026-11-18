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
    cudaStream_t stream;      // donde se encolan sus kernels y sus FFT
    cufftHandle r2c, c2r;     // n transformadas de largo n en cada llamada
    float *work;              // n*n, una linea reordenada por fila
    cufftComplex *spec;       // n*(n/2+1), medio espectro de cada linea
    float2 *twiddle;          // (cos, sin) de pi*k/(2n), k = 0..n-1
} DctGpu;

void dct_gpu_init(DctGpu *d, int n, cudaStream_t stream);
void dct_gpu_free(DctGpu *d);
// data esta en la GPU (n*n, fila iy*n+ix) y se transforma en su lugar
void dct_gpu_forward(DctGpu *d, float *data);
void dct_gpu_inverse(DctGpu *d, float *data);

// eventos para medir cada parte sin sincronizar entre medio: la subida de v,
// los kernels de wirelength y su copia de vuelta, y las partes de densidad
enum {
    EV_UP0, EV_UP1,
    EV_WL_END, EV_WL_D2H,
    EV_D0, EV_D_SPLAT, EV_D_POISSON, EV_D_FIELD, EV_D_GRAD, EV_D_D2H,
    GPU_EVENTS
};

struct GpuPlacer {
    int num_cells, num_nets, num_pins;

    // netlist: se sube una sola vez, ya acomodada para el kernel de
    // wirelength (ver wirelength_gpu.cu). Cada pin ocupa un lugar (slot) y los
    // arreglos van separados por campo.
    int num_slots;
    int *slot_cell;           // celda del pin, -1 en los slots de relleno
    float *slot_dx, *slot_dy;
    float *px, *py;           // posicion de cada slot en la iteracion actual
    int num_small, num_big;   // redes de un hilo y redes de un bloque
    int *warp_base;           // primer slot de cada warp de redes chicas
    int *small_deg;           // grado de cada red chica, en orden de hilo
    int *big_start, *big_deg; // primer slot y grado de cada red grande
    float *cell_w, *cell_h;
    unsigned char *cell_fixed;

    // Wirelength y densidad van en dos streams: asi la CPU puede esperar solo
    // el gradiente de wirelength y trabajar con el mientras la densidad sigue
    // en la GPU. Wirelength tiene mas prioridad porque es el que se espera
    // primero.
    cudaStream_t s_wl, s_d;

    // por iteracion: posiciones (2N, se suben una vez para los dos) y un
    // gradiente (2N) para cada uno
    float *v;
    float *grad;
    float *grad_d;

    // sumas parciales por bloque, que el host termina en double: primero las
    // de wirelength (wl_parts) y despues dos por bloque de bins
    int wl_parts, bin_blocks;
    float *partial;
    float *h_partial;
    int h2d_pending;          // falta sumar al perfil la ultima subida de v
    int wl_grad, d_grad;      // si la evaluacion encolada calcula gradiente

    // grilla de densidad
    GridGeom geo;
    float movable_area;
    float *density, *phi, *ex, *ey;
    float *inv_w2;            // 1/(wx^2+wy^2) por bin, 0 en el DC
    DctGpu dct;

    cudaEvent_t ev[GPU_EVENTS];
};

// partes de una evaluacion (gpu.cu, wirelength_gpu.cu, density_gpu.cu): la
// subida de v, encolar cada calculo en su stream y esperar su resultado
void gpu_upload_v(GpuPlacer *g, const float *v);
void wirelength_enqueue(GpuPlacer *g, float gamma, int with_grad);
double wirelength_wait(GpuPlacer *g, float *grad);
void density_enqueue(GpuPlacer *g, int with_grad);
double density_wait(GpuPlacer *g, float *grad, float *overflow);

void density_gpu_init(GpuPlacer *g, const Grid *grid);
void density_gpu_free(GpuPlacer *g);
void wirelength_gpu_init(GpuPlacer *g, const Netlist *nl);
void wirelength_gpu_free(GpuPlacer *g);
// cuantas sumas parciales deja wirelength (una por bloque de redes chicas y
// una por red grande)
int wirelength_gpu_partials(const GpuPlacer *g);

// reserva en la GPU y copia desde el host
void *gpu_upload(const void *src, size_t bytes);

// segundos entre dos eventos que ya se completaron
double gpu_elapsed(cudaEvent_t a, cudaEvent_t b);
// suma al perfil la subida de v si nadie la sumo todavia
void gpu_count_upload(GpuPlacer *g);

#endif
