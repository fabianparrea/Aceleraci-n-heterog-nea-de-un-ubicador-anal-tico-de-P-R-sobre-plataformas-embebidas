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

typedef struct {
    int num_cells;
    int num_nets;
    int num_pins;
    Cell *cells;
    Net *nets;
    Pin *pins;
} Netlist;

// El vector de posiciones v tiene 2N floats con los centros de las celdas:
// v[i] es x y v[N + i] es y, con N = num_cells.
void netlist_get_positions(const Netlist *nl, float *v);
void netlist_set_positions(Netlist *nl, const float *v);

void netlist_free(Netlist *nl);

#endif
