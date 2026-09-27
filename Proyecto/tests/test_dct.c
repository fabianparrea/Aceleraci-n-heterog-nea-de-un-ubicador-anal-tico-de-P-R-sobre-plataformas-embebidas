#include <math.h>
#include "check.h"
#include "dct.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// una constante solo tiene componente DC (las demas cancelan por ortogonalidad)
static void test_dct_constant(void)
{
    float data[16];
    for (int i = 0; i < 16; i++)
        data[i] = 3.0f;

    dct2d_forward(data, 4, 4);
    CHECK_NEAR(data[0], 16 * 3.0, 1e-2);
    for (int i = 1; i < 16; i++)
        CHECK_NEAR(data[i], 0.0, 1e-2);
}

// ida y vuelta debe devolver lo mismo
static void test_dct_roundtrip(void)
{
    float data[64], orig[64];
    for (int i = 0; i < 64; i++) {
        data[i] = (float)(i * 7 % 13) - 6.0f;
        orig[i] = data[i];
    }

    dct2d_forward(data, 8, 8);
    dct2d_inverse(data, 8, 8);
    for (int i = 0; i < 64; i++)
        CHECK_NEAR(data[i], orig[i], 1e-2);
}

// contra la definicion de DCT-II calculada aparte, para no depender de la
// misma tabla que usa la implementacion (ny=1 hace que el 2D se reduzca a 1D)
static double dct2_direct(const float *x, int n, int k)
{
    double sum = 0.0;
    for (int i = 0; i < n; i++)
        sum += x[i] * cos(M_PI / n * (i + 0.5) * k);
    return sum;
}

static void test_dct_against_direct_1d(void)
{
    float orig[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    float data[8];
    for (int i = 0; i < 8; i++)
        data[i] = orig[i];

    dct2d_forward(data, 8, 1);
    for (int k = 0; k < 8; k++)
        CHECK_NEAR(data[k], dct2_direct(orig, 8, k), 1e-2);
}

int main(void)
{
    test_dct_constant();
    test_dct_roundtrip();
    test_dct_against_direct_1d();
    return finish("test_dct");
}
