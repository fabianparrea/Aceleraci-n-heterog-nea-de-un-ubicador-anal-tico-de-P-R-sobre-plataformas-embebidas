#ifndef GRID_H
#define GRID_H

// Los arreglos son de nx * ny, el bin (ix, iy) queda en iy * nx + ix.
typedef struct {
    int nx, ny;
    float x0, y0;       // esquina inferior izquierda de la region
    float bin_w, bin_h;
    float *density;
    float *phi;
    float *ex, *ey;
} Grid;

#endif
