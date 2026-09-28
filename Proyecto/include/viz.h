#ifndef VIZ_H
#define VIZ_H

#include "netlist.h"

// dibuja un rectangulo por celda (rojo las fijas, azul las movibles) en un PPM
// binario (formato P6, sin comprimir, sin librerias) de a lo sumo max_dim pixeles
// de lado. Se puede abrir con casi cualquier visor, o convertir con
// "convert layout.ppm layout.png".
void write_layout_ppm(const char *path, const Netlist *nl, const float *v, int max_dim);

#endif
