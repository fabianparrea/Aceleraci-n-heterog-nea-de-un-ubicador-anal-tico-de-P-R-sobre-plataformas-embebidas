#include <string.h>
#include "check.h"
#include "bookshelf.h"

static void write_file(const char *path, const char *text)
{
    FILE *f = fopen(path, "w");
    if (f) {
        fputs(text, f);
        fclose(f);
    }
}

// Prueba una red mala junto con los .nodes y .pl del toy. Debe fallar.
static int read_bad_nets(const char *nets_text)
{
    Netlist nl;
    write_file("build/bad.aux",
               "RowBasedPlacement : bad.nets ../bench/toy/toy.nodes ../bench/toy/toy.pl\n");
    write_file("build/bad.nets", nets_text);
    return bookshelf_read("build/bad.aux", &nl);
}

static void test_toy(void)
{
    Netlist nl;
    if (bookshelf_read("bench/toy/toy.aux", &nl) != 0) {
        CHECK(0 && "no se pudo leer el toy");
        return;
    }

    CHECK(nl.num_cells == 8);
    CHECK(nl.num_nets == 4);
    CHECK(nl.num_pins == 11);

    const Cell *c5 = &nl.cells[5];
    CHECK(strcmp(c5->name, "c5") == 0);
    CHECK_NEAR(c5->width, 6, 1e-6);
    CHECK_NEAR(c5->height, 2, 1e-6);
    CHECK_NEAR(c5->area, 12, 1e-6);
    CHECK_NEAR(c5->x, 12, 1e-6);
    CHECK_NEAR(c5->y, 12, 1e-6);
    CHECK(c5->fixed == 0);
    CHECK(nl.cells[6].fixed == 1);
    CHECK(nl.cells[7].fixed == 1);

    CHECK(nl.nets[2].pin_start == 5);
    CHECK(nl.nets[2].degree == 3);
    CHECK(nl.pins[7].cell == 5);
    CHECK(nl.pins[7].net == 2);
    CHECK_NEAR(nl.pins[7].dx, -2, 1e-6);
    CHECK_NEAR(nl.pins[4].dx, -1, 1e-6);
    CHECK_NEAR(nl.pins[4].dy, 1, 1e-6);

    // posiciones: centros en el vector v
    float v[16];
    netlist_get_positions(&nl, v);
    CHECK_NEAR(v[0], 2, 1e-6);
    CHECK_NEAR(v[8 + 0], 1, 1e-6);
    CHECK_NEAR(v[7], 20.5, 1e-6);
    CHECK_NEAR(v[8 + 7], 0.5, 1e-6);

    v[3] += 5;
    netlist_set_positions(&nl, v);
    CHECK_NEAR(nl.cells[3].x, 13, 1e-6);
    CHECK_NEAR(nl.cells[3].y, 8, 1e-6);

    netlist_free(&nl);
}

static void test_errors(void)
{
    Netlist nl;
    CHECK(bookshelf_read("bench/toy/no_existe.aux", &nl) == -1);

    // pin a una celda que no existe
    CHECK(read_bad_nets("NumNets : 1\nNumPins : 2\n"
                        "NetDegree : 2 n0\n c0 B : 0 0\n zz B : 0 0\n") == -1);
    // NumPins no coincide con los pines que hay
    CHECK(read_bad_nets("NumNets : 1\nNumPins : 3\n"
                        "NetDegree : 2 n0\n c0 B : 0 0\n c1 B : 0 0\n") == -1);
    // a la red le faltan pines
    CHECK(read_bad_nets("NumNets : 1\nNumPins : 2\n"
                        "NetDegree : 2 n0\n c0 B : 0 0\n") == -1);
}

int main(void)
{
    test_toy();
    test_errors();
    return finish("test_bookshelf");
}
