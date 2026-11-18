#include <math.h>
#include <stdlib.h>
#include "check.h"
#include "median.h"

static int cmp_float(const void *a, const void *b)
{
    float x = *(const float *)a, y = *(const float *)b;
    return (x > y) - (x < y);
}

// compara contra ordenar las magnitudes con qsort, que es como se hacia antes
static void check_against_sort(const float *v, int n)
{
    float *s = malloc(n * sizeof(float));
    for (int i = 0; i < n; i++)
        s[i] = fabsf(v[i]);
    qsort(s, n, sizeof(float), cmp_float);
    int ks[4] = {0, n / 2, n - 1, n / 3};
    for (int j = 0; j < 4; j++)
        CHECK(select_abs_kth(v, n, ks[j]) == s[ks[j]]);
    free(s);
}

int main(void)
{
    srand(7);
    int n = 400000;
    float *v = malloc(n * sizeof(float));

    // gradientes tipicos: signos mezclados y varios ordenes de magnitud
    for (int i = 0; i < n; i++)
        v[i] = (rand() % 2 ? 1 : -1) * powf(10.0f, -6.0f + 8.0f * rand() / (float)RAND_MAX);
    check_against_sort(v, n);

    // muchos repetidos y ceros (celdas fijas o sin redes)
    for (int i = 0; i < n; i++)
        v[i] = i % 3 == 0 ? 0.0f : (float)(rand() % 5) - 2.0f;
    check_against_sort(v, n);

    // arreglos chicos, que no se reparten entre hilos
    float small[5] = {-3.0f, 1.0f, 0.5f, -0.25f, 2.0f};
    check_against_sort(small, 5);
    CHECK(median_abs(small, 5) == 1.0f);
    float one[1] = {-4.0f};
    CHECK(median_abs(one, 1) == 4.0f);

    free(v);
    return finish("test_median");
}
