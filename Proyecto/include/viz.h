#ifndef VIZ_H
#define VIZ_H

#include "netlist.h"

// dibuja un rectangulo por celda (rojo las fijas, azul las movibles) en un PPM
// binario (formato P6, sin comprimir, sin librerias) de a lo sumo max_dim pixeles
// de lado. Se puede abrir con casi cualquier visor, o convertir con
// "convert layout.ppm layout.png".
void write_layout_ppm(const char *path, const Netlist *nl, const float *v, int max_dim);

// Cuadros para animar el colocado. Todos usan el mismo recuadro (el area de las
// filas), asi el chip no cambia de escala de un cuadro al otro. Las movibles se
// pintan segun cuanto tapan cada pixel: azul mientras caben y amarillo cuando
// hay mas celdas que espacio; las fijas van en rojo. Junto a los PPM queda
// cuadros.csv con la etapa, la iteracion, el HPWL y el overflow de cada uno,
// que es lo que usa tests/make_gif.py para armar el GIF.
typedef struct VizFrames VizFrames;

// crea la carpeta si no existe; devuelve NULL si no se puede escribir ahi
VizFrames *viz_frames_open(const char *dir, const char *name, const Netlist *nl,
                           int every, int max_dim);
// 1 si a esta iteracion le toca cuadro (f puede ser NULL: nunca le toca)
int viz_frames_due(const VizFrames *f, int iter);
// iter < 0 u overflow < 0 quedan vacios en el CSV (por ejemplo, al legalizar)
void viz_frames_add(VizFrames *f, const float *v, const char *stage, int iter,
                    double hpwl, float overflow);
void viz_frames_close(VizFrames *f);

#endif
