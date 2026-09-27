#ifndef PLACE_H
#define PLACE_H

#include "netlist.h"

// Corre el ciclo de optimizacion (wirelength + densidad, con momento de Nesterov)
// y deja las celdas movibles en su posicion final (sin legalizar) dentro de nl.
// out_overflow, si no es NULL, recibe el overflow del punto que se dejo en nl.
void run_placement(Netlist *nl, float *out_overflow);

#endif
