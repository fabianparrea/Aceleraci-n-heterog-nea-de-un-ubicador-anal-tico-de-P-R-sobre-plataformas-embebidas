#include <stdio.h>
#include <stdlib.h>
#include "bookshelf.h"
#include "wirelength.h"

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "uso: %s benchmark.aux\n", argv[0]);
        return 1;
    }

    Netlist nl;
    if (bookshelf_read(argv[1], &nl) < 0)
        return 1;

    float *v = malloc(2 * nl.num_cells * sizeof(float));
    if (!v) {
        netlist_free(&nl);
        return 1;
    }
    netlist_get_positions(&nl, v);

    printf("celdas: %d  redes: %d  pines: %d\n", nl.num_cells, nl.num_nets, nl.num_pins);
    printf("HPWL inicial: %.6e\n", compute_hpwl(&nl, v));

    free(v);
    netlist_free(&nl);
    return 0;
}
