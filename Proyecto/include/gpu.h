#ifndef GPU_H
#define GPU_H

#include "netlist.h"
#include "grid.h"

#ifdef __cplusplus
extern "C" {
#endif

// Version CUDA de los dos calculos pesados del ciclo: wirelength y densidad
// (reparto de area, Poisson con DCT via cuFFT, campo y gradiente). Mismas
// cuentas que compute_wirelength y compute_density: v entra desde el host y
// los gradientes vuelven al host.
typedef struct GpuPlacer GpuPlacer;

// Sube lo que no cambia entre iteraciones (redes, pines, tamano de las celdas,
// la grilla) y reserva los buffers. NULL si no hay GPU.
GpuPlacer *gpu_create(const Netlist *nl, const Grid *grid);
void gpu_destroy(GpuPlacer *g);

// Una evaluacion por iteracion, en dos mitades para que la CPU no se quede
// esperando: gpu_eval_start sube v una sola vez y deja encolados wirelength y
// densidad, cada uno en su stream, y vuelve enseguida. Cada wait espera solo
// su parte y trae su gradiente. v no se puede modificar hasta los dos wait.
void gpu_eval_start(GpuPlacer *g, const float *v, float gamma);
double gpu_wait_wirelength(GpuPlacer *g, float *grad);
double gpu_wait_density(GpuPlacer *g, float *grad, float *overflow);

// cada calculo por separado y esperando el resultado (para las pruebas)
double gpu_wirelength(GpuPlacer *g, const float *v, float gamma, float *grad);
double gpu_density(GpuPlacer *g, const float *v, float *grad, float *overflow);

// DCT-II 2D de n*n (inverse = 0) o su inversa, en la GPU sobre datos del host.
// Hace lo mismo que dct2d_forward/dct2d_inverse; esta para poder probarla sola.
int gpu_dct2d(float *data, int n, int inverse);

#ifdef __cplusplus
}
#endif

#endif
