#include "check.h"
#include "legalize.h"

// dos celdas con el mismo destino (se traslapan del todo): un barrido simple
// dejaria una en su lugar y empujaria la otra el doble de lejos; Abacus las
// reparte parejo alrededor del punto que las dos querian, moviendo menos en
// total (se puede sacar a mano resolviendo el promedio con ancho acumulado,
// ver el comentario de Cluster en legalize.c).
static void test_abacus_balances_displacement(void)
{
    Cell cells[2] = {
        {0, NULL, 2, 2, 0, 0, 4, 0},
        {1, NULL, 2, 2, 0, 0, 4, 0},
    };
    Row rows[1] = {{ -10, 0, 2, 1, 1000 }};   // x0, y0, height, site_w, num_sites
    Netlist nl = {2, 0, 0, cells, NULL, NULL, 1, rows};

    float v[4] = {1, 1, 1, 1};   // las dos centradas en x=0 (corner 0, ancho 2)
    legalize(&nl, v);

    CHECK_NEAR(v[0], 0.0, 1e-3);   // celda 0 queda centrada en x=-1..1 -> centro 0
    CHECK_NEAR(v[1], 2.0, 1e-3);   // celda 1 justo despues, centro en 2
    CHECK_NEAR(v[2], 1.0, 1e-6);   // y no se toca
    CHECK_NEAR(v[3], 1.0, 1e-6);
}

// una celda movible cuyo destino cae encima de una fija: se tiene que correr
// hasta despues del borde de la fija, no quedar traslapada con ella.
static void test_avoids_fixed_obstacle(void)
{
    Cell cells[2] = {
        {0, NULL, 4, 2, 0, 0, 8, 1},   // fija, ocupa x en [0,4)
        {1, NULL, 2, 2, 0, 0, 4, 0},   // movible, quiere el mismo lugar
    };
    Row rows[1] = {{0, 0, 2, 1, 1000}};
    Netlist nl = {2, 0, 0, cells, NULL, NULL, 1, rows};

    float v[4] = {2, 1, 1, 1};   // fija centrada en x=2 (corner 0..4), movible quiere x=0..2
    legalize(&nl, v);

    CHECK_NEAR(v[0], 2.0, 1e-6);   // la fija no se mueve
    CHECK(v[1] - 1.0f >= 4.0f - 1e-3f);   // la movible (corner = v[1]-1) queda en x>=4
}

int main(void)
{
    test_abacus_balances_displacement();
    test_avoids_fixed_obstacle();
    return finish("test_legalize");
}
