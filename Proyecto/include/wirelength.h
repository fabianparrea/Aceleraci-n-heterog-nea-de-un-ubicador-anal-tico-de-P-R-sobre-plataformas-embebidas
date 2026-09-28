#ifndef WIRELENGTH_H
#define WIRELENGTH_H

#include "netlist.h"

// HPWL exacto, para reportar la calidad final
double compute_hpwl(const Netlist *nl, const float *v);

// Wirelength suave (weighted-average). Llena grad (2N floats) y devuelve el total.
// gamma esta en unidades de coordenada. grad puede ser NULL.
double compute_wirelength(const Netlist *nl, const float *v, float gamma, float *grad);

#endif
