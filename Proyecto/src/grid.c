#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "grid.h"

// reservado alineado a 64 bytes (una linea de cache, y multiplo del ancho de
// un registro AVX) para que el compilador pueda vectorizar los bucles de
// density.c/dct.c sin tener que revisar la alineacion en cada acceso.
static float *aligned_zeros(size_t n)
{
    void *p;
    if (posix_memalign(&p, 64, n * sizeof(float)) != 0)
        return NULL;
    memset(p, 0, n * sizeof(float));
    return p;
}

static int next_pow2(int x)
{
    int p = 1;
    while (p < x)
        p <<= 1;
    return p;
}

// cuantos bins por lado usar: apuntamos a un promedio de unas 8 celdas por bin,
// acotado para que la grilla no se quede muy chica ni crezca demasiado.
static int choose_bin_count(int num_cells)
{
    int target_bins = num_cells / 8;
    if (target_bins < 1)
        target_bins = 1;

    int side = next_pow2((int)ceil(sqrt((double)target_bins)));
    if (side < 16)
        side = 16;
    if (side > 512)
        side = 512;
    return side;
}

void grid_init(Grid *grid, const Netlist *nl)
{
    float xmin = FLT_MAX, xmax = -FLT_MAX;
    float ymin = FLT_MAX, ymax = -FLT_MAX;
    for (int i = 0; i < nl->num_cells; i++) {
        const Cell *c = &nl->cells[i];
        if (c->x < xmin) xmin = c->x;
        if (c->x + c->width > xmax) xmax = c->x + c->width;
        if (c->y < ymin) ymin = c->y;
        if (c->y + c->height > ymax) ymax = c->y + c->height;
    }

    // un margen chico para que ninguna celda quede pegada exactamente al borde
    // de la grilla (ahi el overlap con el bin se recorta en vez de repartirse
    // con un vecino, y la derivada deja de estar bien definida).
    float pad_x = (xmax - xmin) * 0.01f;
    float pad_y = (ymax - ymin) * 0.01f;
    xmin -= pad_x; xmax += pad_x;
    ymin -= pad_y; ymax += pad_y;

    int side = choose_bin_count(nl->num_cells);
    grid->nx = side;
    grid->ny = side;
    grid->x0 = xmin;
    grid->y0 = ymin;
    grid->bin_w = (xmax - xmin) / side;
    grid->bin_h = (ymax - ymin) / side;

    double movable_area = 0.0;
    for (int i = 0; i < nl->num_cells; i++)
        if (!nl->cells[i].fixed)
            movable_area += nl->cells[i].area;
    grid->movable_area = (float)movable_area;

    size_t nb = (size_t)grid->nx * grid->ny;
    grid->density = aligned_zeros(nb);
    grid->phi = aligned_zeros(nb);
    grid->ex = aligned_zeros(nb);
    grid->ey = aligned_zeros(nb);
}

void grid_free(Grid *grid)
{
    free(grid->density);
    free(grid->phi);
    free(grid->ex);
    free(grid->ey);
}
