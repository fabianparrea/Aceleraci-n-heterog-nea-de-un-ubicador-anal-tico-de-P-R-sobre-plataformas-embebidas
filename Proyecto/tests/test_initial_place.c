#include "check.h"
#include "initial_place.h"

// dos resortes iguales tirando de una celda movible desde 0 y desde 10: en
// equilibrio queda justo en el medio. Sale de igualar a cero la derivada de
// la energia (ver comentario de apply_laplacian en initial_place.c).
static void test_two_springs(void)
{
    Cell cells[3] = {
        {0, NULL, 0, 0, 0, 0, 0, 1},   // f0, fija en x=0
        {1, NULL, 0, 0, 0, 0, 0, 0},   // m0, movible
        {2, NULL, 0, 0, 0, 0, 0, 1},   // f1, fija en x=10
    };
    Pin pins[4] = {
        {0, 0, 0, 0}, {1, 0, 0, 0},    // red 0: f0-m0
        {1, 1, 0, 0}, {2, 1, 0, 0},    // red 1: m0-f1
    };
    Net nets[2] = {{0, 0, 2}, {1, 2, 2}};
    Netlist nl = {3, 2, 4, cells, nets, pins, 0, NULL};

    float v[6] = {0, 3, 10, 0, 0, 0};   // x0,x1,x2, y0,y1,y2
    initial_place(&nl, v);

    CHECK_NEAR(v[1], 5.0, 1e-3);
    CHECK_NEAR(v[0], 0.0, 1e-6);   // las fijas no se mueven
    CHECK_NEAR(v[2], 10.0, 1e-6);
}

// una sola red de 3 pines (dos fijas y una movible) deja a la movible en el
// centroide de las fijas -- propiedad conocida del modelo de resorte tipo
// estrella con una unica red.
static void test_single_net_three_pins(void)
{
    Cell cells[3] = {
        {0, NULL, 0, 0, 0, 0, 0, 1},   // f0 en x=0
        {1, NULL, 0, 0, 0, 0, 0, 0},   // m0, movible
        {2, NULL, 0, 0, 0, 0, 0, 1},   // f1 en x=12
    };
    Pin pins[3] = {{0, 0, 0, 0}, {1, 0, 0, 0}, {2, 0, 0, 0}};
    Net nets[1] = {{0, 0, 3}};
    Netlist nl = {3, 1, 3, cells, nets, pins, 0, NULL};

    float v[6] = {0, 20, 12, 0, 0, 0};
    initial_place(&nl, v);

    CHECK_NEAR(v[1], 6.0, 1e-3);
}

int main(void)
{
    test_two_springs();
    test_single_net_three_pins();
    return finish("test_initial_place");
}
