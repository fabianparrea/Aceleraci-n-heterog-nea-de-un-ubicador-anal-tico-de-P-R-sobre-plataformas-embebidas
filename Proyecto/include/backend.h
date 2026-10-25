#ifndef BACKEND_H
#define BACKEND_H

#include "netlist.h"
#include "grid.h"

// Donde corren los dos calculos pesados de cada iteracion (wirelength y
// densidad): en la CPU, o en la GPU si se compilo la version CUDA (make gpu).
// place.c llama siempre a estas funciones sin saber cual de las dos es.
typedef struct Backend Backend;

// NULL si es la version CUDA y no hay ninguna GPU disponible
Backend *backend_create(const Netlist *nl, Grid *grid);
void backend_destroy(Backend *b);

// mismo contrato que compute_wirelength y compute_density
double backend_wirelength(Backend *b, const float *v, float gamma, float *grad);
double backend_density(Backend *b, const float *v, float *grad, float *overflow);

#endif
