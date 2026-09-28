#ifndef NETLIST_H
#define NETLIST_H

typedef struct {
    int id;
    char *name;
    float width, height;
    float x, y;         // esquina inferior izquierda
    float area;
    int fixed;
} Cell;

typedef struct {
    int cell;           // indice en nl->cells
    int net;            // indice en nl->nets
    float dx, dy;       // offset desde el centro de la celda
} Pin;

typedef struct {
    int id;
    int pin_start;      // primer pin de la red en nl->pins
    int degree;
} Net;

// una fila del chip (del .scl): las celdas se legalizan alineadas a esto
typedef struct {
    float x0, y0;        // esquina inferior izquierda de la fila
    float height;
    float site_w;        // ancho de un sitio: la x legal es x0 + k*site_w
    int num_sites;        // la fila va de x0 a x0 + num_sites*site_w
} Row;

typedef struct {
    int num_cells;
    int num_nets;
    int num_pins;
    Cell *cells;
    Net *nets;
    Pin *pins;
    int num_rows;        // 0 si el benchmark no trae .scl (no se puede legalizar)
    Row *rows;
} Netlist;

// El vector de posiciones v tiene 2N floats con los centros de las celdas:
// v[i] es x y v[N + i] es y, con N = num_cells.
void netlist_get_positions(const Netlist *nl, float *v);
void netlist_set_positions(Netlist *nl, const float *v);

void netlist_free(Netlist *nl);

#endif
