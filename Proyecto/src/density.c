#include <math.h>
#include <string.h>
#include "density.h"
#include "dct.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static float overlap_1d(float a0, float a1, float b0, float b1)
{
    float lo = a0 > b0 ? a0 : b0;
    float hi = a1 < b1 ? a1 : b1;
    return hi > lo ? hi - lo : 0.0f;
}

// reparte el area de la celda i entre los bins que toca (overlap exacto de rectangulo)
static void cell_bin_range(const Netlist *nl, const Grid *grid, int i, const float *v,
                           float *x0, float *x1, float *y0, float *y1,
                           int *ix0, int *ix1, int *iy0, int *iy1)
{
    int n = nl->num_cells;
    const Cell *c = &nl->cells[i];
    float cx = v[i], cy = v[n + i];
    *x0 = cx - c->width / 2;
    *x1 = cx + c->width / 2;
    *y0 = cy - c->height / 2;
    *y1 = cy + c->height / 2;

    *ix0 = (int)floorf((*x0 - grid->x0) / grid->bin_w);
    *ix1 = (int)ceilf((*x1 - grid->x0) / grid->bin_w);
    *iy0 = (int)floorf((*y0 - grid->y0) / grid->bin_h);
    *iy1 = (int)ceilf((*y1 - grid->y0) / grid->bin_h);

    // si la celda quedo total o parcialmente fuera de la grilla (puede pasar si
    // un paso del optimizador la manda lejos) el rango se recorta de los dos
    // lados, no solo de uno, para que nunca cruce el limite del arreglo.
    if (*ix0 < 0) *ix0 = 0;
    if (*ix0 > grid->nx) *ix0 = grid->nx;
    if (*ix1 < 0) *ix1 = 0;
    if (*ix1 > grid->nx) *ix1 = grid->nx;

    if (*iy0 < 0) *iy0 = 0;
    if (*iy0 > grid->ny) *iy0 = grid->ny;
    if (*iy1 < 0) *iy1 = 0;
    if (*iy1 > grid->ny) *iy1 = grid->ny;
}

static void splat_density(const Netlist *nl, Grid *grid, const float *v)
{
    memset(grid->density, 0, (size_t)grid->nx * grid->ny * sizeof(float));

    // celdas distintas pueden caer en el mismo bin, por eso el incremento es
    // atomico: es la unica parte de esta funcion que se comparte entre celdas.
    #pragma omp parallel for if(nl->num_cells > 1000)
    for (int i = 0; i < nl->num_cells; i++) {
        float x0, x1, y0, y1;
        int ix0, ix1, iy0, iy1;
        cell_bin_range(nl, grid, i, v, &x0, &x1, &y0, &y1, &ix0, &ix1, &iy0, &iy1);

        for (int iy = iy0; iy < iy1; iy++) {
            float by0 = grid->y0 + iy * grid->bin_h;
            float oy = overlap_1d(y0, y1, by0, by0 + grid->bin_h);
            if (oy <= 0)
                continue;
            for (int ix = ix0; ix < ix1; ix++) {
                float bx0 = grid->x0 + ix * grid->bin_w;
                float ox = overlap_1d(x0, x1, bx0, bx0 + grid->bin_w);
                if (ox <= 0)
                    continue;
                #pragma omp atomic
                grid->density[iy * grid->nx + ix] += ox * oy;
            }
        }
    }
}

// resuelve la ecuacion de Poisson (densidad -> potencial) via DCT: en frecuencia,
// dividir por (wx^2+wy^2) es lo mismo que aplicar el laplaciano inverso.
static void solve_poisson(Grid *grid)
{
    int nx = grid->nx, ny = grid->ny;
    memcpy(grid->phi, grid->density, (size_t)nx * ny * sizeof(float));
    dct2d_forward(grid->phi, nx, ny);

    // no se reparte entre hilos: es una division por bin, muy poco trabajo
    // para lo que cuesta abrir un bloque paralelo (medido: sale mas lento).
    for (int iy = 0; iy < ny; iy++) {
        double wy = M_PI * iy / ny;
        for (int ix = 0; ix < nx; ix++) {
            double wx = M_PI * ix / nx;
            int idx = iy * nx + ix;
            if (ix == 0 && iy == 0) {
                grid->phi[idx] = 0.0f;   // el nivel base del potencial no importa
                continue;
            }
            grid->phi[idx] = (float)(grid->phi[idx] / (wx * wx + wy * wy));
        }
    }
    dct2d_inverse(grid->phi, nx, ny);
}

// campo electrico = -gradiente del potencial, por diferencias finitas
// centradas. Es una version suavizada del potencial en cada bin: asi todas
// las celdas sienten una fuerza, no solo las que cruzan justo un borde de bin
// (que con bins mucho mas grandes que una celda serian casi todas en cero).
static void compute_field(Grid *grid)
{
    int nx = grid->nx, ny = grid->ny;
    // se probo repartir esto entre hilos (cada bin es independiente, no hacia
    // falta atomico) pero salio mas lento: es solo una resta y una division
    // por bin, el overhead de abrir el bloque paralelo cuesta mas de lo que
    // ahorra. Se deja en serie a proposito.
    for (int iy = 0; iy < ny; iy++) {
        for (int ix = 0; ix < nx; ix++) {
            int il = ix > 0 ? ix - 1 : ix;
            int ir = ix < nx - 1 ? ix + 1 : ix;
            int ib = iy > 0 ? iy - 1 : iy;
            int it = iy < ny - 1 ? iy + 1 : iy;

            float dx = (ir - il) * grid->bin_w;
            float dy = (it - ib) * grid->bin_h;
            grid->ex[iy * nx + ix] = -(grid->phi[iy * nx + ir] - grid->phi[iy * nx + il]) / dx;
            grid->ey[iy * nx + ix] = -(grid->phi[it * nx + ix] - grid->phi[ib * nx + ix]) / dy;
        }
    }
}

double compute_density(const Netlist *nl, Grid *grid, const float *v,
                       float *grad, float *overflow)
{
    splat_density(nl, grid, v);
    solve_poisson(grid);
    compute_field(grid);

    // estas dos sumas tambien se probaron en paralelo (son reducciones
    // limpias, sin atomico) pero perdieron contra la version en serie: muy
    // poco trabajo por bin para lo que cuesta combinar el resultado de cada
    // hilo al final.
    int nb = grid->nx * grid->ny;
    double energy = 0.0;
    for (int b = 0; b < nb; b++)
        energy += (double)grid->density[b] * grid->phi[b];
    energy *= 0.5;

    double bin_cap = (double)grid->bin_w * grid->bin_h;   // target de ocupacion = 100%
    double over = 0.0;
    for (int b = 0; b < nb; b++) {
        double excess = grid->density[b] - bin_cap;
        if (excess > 0)
            over += excess;
    }
    // el area movible no depende de la posicion: se calcula una sola vez en
    // grid_init en vez de sumarla de nuevo en cada llamada.
    if (overflow)
        *overflow = grid->movable_area > 0 ? (float)(over / grid->movable_area) : 0.0f;

    if (grad) {
        memset(grad, 0, 2 * (size_t)nl->num_cells * sizeof(float));
        int n = nl->num_cells;
        // cada celda escribe solo su propio par grad[i]/grad[n+i], nunca el
        // de otra, asi que no hace falta atomico para repartir este bucle.
        #pragma omp parallel for if(n > 1000)
        for (int i = 0; i < n; i++) {
            if (nl->cells[i].fixed)
                continue;
            float x0, x1, y0, y1;
            int ix0, ix1, iy0, iy1;
            cell_bin_range(nl, grid, i, v, &x0, &x1, &y0, &y1, &ix0, &ix1, &iy0, &iy1);
            if (ix1 <= ix0 || iy1 <= iy0)
                continue;   // la celda quedo totalmente fuera de la grilla

            // la celda "siente" el promedio del campo en los bins que toca,
            // pesado por cuanto la cubre cada uno (como una carga repartida).
            double gx = 0.0, gy = 0.0;
            for (int iy = iy0; iy < iy1; iy++) {
                float by0 = grid->y0 + iy * grid->bin_h;
                float oy = overlap_1d(y0, y1, by0, by0 + grid->bin_h);
                if (oy <= 0)
                    continue;
                for (int ix = ix0; ix < ix1; ix++) {
                    float bx0 = grid->x0 + ix * grid->bin_w;
                    float ox = overlap_1d(x0, x1, bx0, bx0 + grid->bin_w);
                    if (ox <= 0)
                        continue;
                    int idx = iy * grid->nx + ix;
                    double area = (double)ox * oy;
                    gx -= area * grid->ex[idx];
                    gy -= area * grid->ey[idx];
                }
            }

            grad[i] = (float)gx;
            grad[n + i] = (float)gy;
        }
    }
    return energy;
}
