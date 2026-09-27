#ifndef GRID_H
#define GRID_H

#include "netlist.h"

// Los arreglos son de nx * ny, el bin (ix, iy) queda en iy * nx + ix.
typedef struct {
    int nx, ny;
    float x0, y0;       // esquina inferior izquierda de la region
    float bin_w, bin_h;
    float *density;
    float *phi;
    float *ex, *ey;
    float movable_area;   // suma del area de las celdas no fijas; no cambia
                           // con la posicion, se calcula una sola vez aca.
} Grid;

// Arma la grilla a partir del bounding box de todas las celdas (fijas incluidas,
// que en bookshelf suelen marcar el borde del die). nx y ny quedan iguales y en
// potencia de 2.
void grid_init(Grid *grid, const Netlist *nl);
void grid_free(Grid *grid);

#endif
