#include <math.h>
#include "check.h"
#include "bookshelf.h"
#include "wirelength.h"

static void test_hpwl_toy(void)
{
    Netlist nl;
    float v[16];
    if (bookshelf_read("bench/toy/toy.aux", &nl) != 0) {
        CHECK(0 && "no se pudo leer el toy");
        return;
    }
    netlist_get_positions(&nl, v);

    CHECK_NEAR(compute_hpwl(&nl, v), 73.0, 1e-4);
    netlist_free(&nl);
}

// Con dos pines el WA es d * tanh(d / (2 * gamma)), se puede calcular a mano
static void test_wa_two_pins(void)
{
    Cell cells[2] = {{0}};
    Pin pins[2] = {{0, 0, 0, 0}, {1, 0, 0, 0}};
    Net nets[1] = {{0, 0, 2}};
    Netlist nl = {2, 1, 2, cells, nets, pins, 0, NULL};

    float v[4] = {0, 10, 0, 0};     // x0, x1, y0, y1
    float grad[4];
    double wa = compute_wirelength(&nl, v, 5.0f, grad);

    CHECK_NEAR(wa, 7.615941559, 1e-4);
    CHECK_NEAR(grad[0], -1.181568405, 1e-4);
    CHECK_NEAR(grad[1], 1.181568405, 1e-4);
    CHECK_NEAR(grad[2], 0, 1e-4);
    CHECK_NEAR(grad[3], 0, 1e-4);
}

static void test_wa_toy(void)
{
    Netlist nl;
    float v[16], grad[16], vt[16], tmp[16];
    if (bookshelf_read("bench/toy/toy.aux", &nl) != 0) {
        CHECK(0 && "no se pudo leer el toy");
        return;
    }
    netlist_get_positions(&nl, v);

    // el WA queda por debajo del HPWL y se le acerca con gamma chico
    CHECK(compute_wirelength(&nl, v, 2.0f, grad) < 73.0);
    CHECK_NEAR(compute_wirelength(&nl, v, 0.01f, grad), 73.0, 1e-2);

    // mover todo junto no cambia el wirelength, asi que el gradiente suma cero
    compute_wirelength(&nl, v, 2.0f, grad);
    double sum_x = 0, sum_y = 0;
    for (int i = 0; i < 8; i++) {
        sum_x += grad[i];
        sum_y += grad[8 + i];
    }
    CHECK_NEAR(sum_x, 0, 1e-4);
    CHECK_NEAR(sum_y, 0, 1e-4);

    // gradiente contra diferencias finitas
    float h = 0.05f;
    for (int i = 0; i < 16; i++) {
        for (int k = 0; k < 16; k++)
            vt[k] = v[k];
        vt[i] = v[i] + h;
        double up = compute_wirelength(&nl, vt, 2.0f, tmp);
        vt[i] = v[i] - h;
        double down = compute_wirelength(&nl, vt, 2.0f, tmp);
        CHECK_NEAR(grad[i], (up - down) / (2 * h), 5e-3);
    }
    netlist_free(&nl);
}

int main(void)
{
    test_hpwl_toy();
    test_wa_two_pins();
    test_wa_toy();
    return finish("test_wirelength");
}
