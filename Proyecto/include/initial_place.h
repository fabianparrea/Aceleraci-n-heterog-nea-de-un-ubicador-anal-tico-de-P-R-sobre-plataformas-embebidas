#ifndef INITIAL_PLACE_H
#define INITIAL_PLACE_H

#include "netlist.h"

// Colocacion inicial por minimos cuadrados: cada red conecta sus celdas con
// un resorte, y resuelve donde quedan las movibles en equilibrio (las fijas
// no se mueven, tiran de las demas). No sabe nada de densidad ni de
// traslapes, solo de conectividad -- sirve para arrancar el loop principal
// desde una posicion razonable en vez de amontonada o al azar.
void initial_place(const Netlist *nl, float *v);

#endif
