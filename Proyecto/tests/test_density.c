#include <math.h>
#include "check.h"
#include "bookshelf.h"
#include "density.h"
#include "grid.h"

// el gradiente de densidad es el campo local promediado por celda, no la
// derivada exacta del overlap (por eso no se compara contra diferencias
// finitas como wirelength): con bins mucho mas grandes que una celda, esa
// derivada exacta da cero para casi todas las celdas menos las que cruzan
// justo un borde de bin. Lo que si tiene que cumplir siempre es la propiedad
// fisica basica: una celda metida entre otras se tiene que sentir empujada
// lejos de donde esta lo mas amontonado.
static void test_density_pushes_apart(void)
{
    Netlist nl;
    if (bookshelf_read("bench/toy/toy.aux", &nl) != 0) {
        CHECK(0 && "no se pudo leer el toy");
        return;
    }
    Grid grid;
    grid_init(&grid, &nl);
    int n = nl.num_cells;

    // todas las celdas amontonadas en el centro de la grilla, menos la 0, que
    // queda un poco a la izquierda del resto
    float cx = grid.x0 + grid.bin_w * grid.nx / 2.0f;
    float cy = grid.y0 + grid.bin_h * grid.ny / 2.0f;
    float v[16], grad[16];
    for (int i = 0; i < n; i++) {
        v[i] = cx;
        v[n + i] = cy;
    }
    v[0] = cx - grid.bin_w;

    float overflow;
    compute_density(&nl, &grid, v, grad, &overflow);
    CHECK(overflow >= 0.0f);

    // el resto del montón queda a la derecha de la celda 0: la tiene que
    // empujar mas a la izquierda todavia (bajar x, o sea gradiente positivo,
    // porque el paso del optimizador se da como v - paso*gradiente)
    CHECK(grad[0] > 0.0f);

    grid_free(&grid);
    netlist_free(&nl);
}

// amontonar las celdas tiene que dar mas overflow que dejarlas como en el toy
static void test_density_overflow_direction(void)
{
    Netlist nl;
    if (bookshelf_read("bench/toy/toy.aux", &nl) != 0) {
        CHECK(0 && "no se pudo leer el toy");
        return;
    }
    Grid grid;
    grid_init(&grid, &nl);
    int n = nl.num_cells;

    float v_spread[16];
    netlist_get_positions(&nl, v_spread);

    float v_stack[16];
    for (int i = 0; i < n; i++) {
        v_stack[i] = v_spread[0];
        v_stack[n + i] = v_spread[n];
    }

    float overflow_stack, overflow_spread;
    compute_density(&nl, &grid, v_stack, NULL, &overflow_stack);
    compute_density(&nl, &grid, v_spread, NULL, &overflow_spread);

    CHECK(overflow_stack > overflow_spread);

    grid_free(&grid);
    netlist_free(&nl);
}

int main(void)
{
    test_density_pushes_apart();
    test_density_overflow_direction();
    return finish("test_density");
}
