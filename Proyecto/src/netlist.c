#include <stdlib.h>
#include <string.h>
#include "netlist.h"

void netlist_get_positions(const Netlist *nl, float *v)
{
    int n = nl->num_cells;
    for (int i = 0; i < n; i++) {
        const Cell *c = &nl->cells[i];
        v[i] = c->x + c->width / 2;
        v[n + i] = c->y + c->height / 2;
    }
}

void netlist_set_positions(Netlist *nl, const float *v)
{
    int n = nl->num_cells;
    for (int i = 0; i < n; i++) {
        Cell *c = &nl->cells[i];
        c->x = v[i] - c->width / 2;
        c->y = v[n + i] - c->height / 2;
    }
}

void netlist_free(Netlist *nl)
{
    for (int i = 0; i < nl->num_cells; i++)
        free(nl->cells[i].name);
    free(nl->cells);
    free(nl->nets);
    free(nl->pins);
    free(nl->rows);
    memset(nl, 0, sizeof *nl);
}
