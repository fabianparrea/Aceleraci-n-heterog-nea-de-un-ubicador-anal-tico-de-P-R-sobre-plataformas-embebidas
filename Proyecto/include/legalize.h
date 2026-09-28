#ifndef LEGALIZE_H
#define LEGALIZE_H

#include "netlist.h"

// Alinea las celdas movibles a las filas de nl->rows (del .scl), sin traslape entre
// ellas ni con las celdas fijas, y a la grilla de sitios de cada fila en x. Si no
// hay filas (nl->num_rows == 0, benchmark sin .scl) no hace nada.
void legalize(Netlist *nl, float *v);

#endif
