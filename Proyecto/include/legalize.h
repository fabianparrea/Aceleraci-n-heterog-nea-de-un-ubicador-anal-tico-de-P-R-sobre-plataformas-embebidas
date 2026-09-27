#ifndef LEGALIZE_H
#define LEGALIZE_H

#include "netlist.h"

// Alinea las celdas a filas sin traslape. Falta definir como pasar las filas (.scl).
void legalize(Netlist *nl, float *v);

#endif
