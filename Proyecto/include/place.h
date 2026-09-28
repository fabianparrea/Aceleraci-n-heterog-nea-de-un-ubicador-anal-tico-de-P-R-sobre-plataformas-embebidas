#ifndef PLACE_H
#define PLACE_H

#include "netlist.h"

// tiempo de pared (segundos) de cada etapa interna de run_placement, para
// perfilado por instrumentacion. Cualquiera de los dos punteros puede ir NULL.
typedef struct {
    double t_initial_place;
    double t_nesterov;
} PlaceTimings;

// Corre el ciclo de optimizacion (wirelength + densidad, con momento de Nesterov)
// y deja las celdas movibles en su posicion final (sin legalizar) dentro de nl.
// out_overflow, si no es NULL, recibe el overflow del punto que se dejo en nl.
// out_timings, si no es NULL, recibe cuanto tardo cada etapa interna.
void run_placement(Netlist *nl, float *out_overflow, PlaceTimings *out_timings);

#endif
