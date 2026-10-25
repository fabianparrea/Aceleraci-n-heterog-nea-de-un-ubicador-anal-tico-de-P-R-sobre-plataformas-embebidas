#ifndef GPU_H
#define GPU_H

#include "netlist.h"
#include "grid.h"

#ifdef __cplusplus
extern "C" {
#endif

// Version CUDA de los dos calculos pesados del ciclo: wirelength (un hilo por
// red) y densidad (reparto de area, Poisson con DCT via cuFFT, campo y
// gradiente). Mismas cuentas y misma interfaz que compute_wirelength y
// compute_density: v entra desde el host y el gradiente vuelve al host. En
// este prototipo cada llamada copia sus datos de ida y vuelta; optimizar esas
// transferencias queda para la siguiente etapa.
typedef struct GpuPlacer GpuPlacer;

// Sube lo que no cambia entre iteraciones (redes, pines, tamano de las celdas,
// la grilla) y reserva los buffers. NULL si no hay GPU.
GpuPlacer *gpu_create(const Netlist *nl, const Grid *grid);
void gpu_destroy(GpuPlacer *g);

double gpu_wirelength(GpuPlacer *g, const float *v, float gamma, float *grad);
double gpu_density(GpuPlacer *g, const float *v, float *grad, float *overflow);

// DCT-II 2D de n*n (inverse = 0) o su inversa, en la GPU sobre datos del host.
// Hace lo mismo que dct2d_forward/dct2d_inverse; esta para poder probarla sola.
int gpu_dct2d(float *data, int n, int inverse);

#ifdef __cplusplus
}
#endif

#endif
