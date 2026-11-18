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

// Wirelength y densidad de un mismo punto v, en dos mitades: eval_start
// arranca el calculo y cada wait devuelve su resultado, con el mismo contrato
// que compute_wirelength y compute_density. En la version CUDA la GPU trabaja
// entre start y los wait, y la CPU puede hacer otra cosa en ese rato; en la
// version CPU los calculos se hacen dentro de cada wait. v tiene que seguir
// igual hasta el ultimo wait.
void backend_eval_start(Backend *b, const float *v, float gamma);
double backend_wait_wirelength(Backend *b, float *grad);
double backend_wait_density(Backend *b, float *grad, float *overflow);

#endif
