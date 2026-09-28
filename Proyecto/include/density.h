#ifndef DENSITY_H
#define DENSITY_H

#include "netlist.h"
#include "grid.h"

// Llena grad (2N floats) y devuelve la energia. El overflow sale por puntero.
double compute_density(const Netlist *nl, Grid *grid, const float *v,
                       float *grad, float *overflow);

#endif
