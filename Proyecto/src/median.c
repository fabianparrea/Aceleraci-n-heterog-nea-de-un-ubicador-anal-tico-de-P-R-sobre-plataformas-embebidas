#include <string.h>
#include "median.h"

// Seleccion por radix sobre los bits del float. Para un float sin signo, el
// orden de los valores es el mismo que el de sus bits leidos como entero, asi
// que el k-esimo menor se puede armar de a pedazos: un histograma de los bits
// altos dice en que rango cae, y las siguientes pasadas fijan los bits que
// faltan mirando solo los que comparten los ya fijados. Son tres pasadas que
// solo leen el arreglo, y cada una se reparte entre los hilos con un
// histograma propio; quickselect en cambio va moviendo datos y no se
// paraleliza bien. Los 31 bits de |v| quedan en 10 + 11 + 10.
#define PASSES 3
static const int pass_shift[PASSES] = {21, 10, 0};
static const int pass_bits[PASSES] = {10, 11, 10};

float select_abs_kth(const float *v, int n, int k)
{
    if (n <= 0 || k < 0 || k >= n)
        return 0.0f;
    unsigned prefix = 0, mask = 0;   // bits ya fijados y cuales son

    for (int p = 0; p < PASSES; p++) {
        int shift = pass_shift[p], nb = 1 << pass_bits[p];
        unsigned hist[1 << 11];
        memset(hist, 0, nb * sizeof(unsigned));

        #pragma omp parallel if(n > 10000)
        {
            unsigned local[1 << 11];
            memset(local, 0, nb * sizeof(unsigned));
            #pragma omp for nowait
            for (int i = 0; i < n; i++) {
                unsigned b;
                memcpy(&b, &v[i], sizeof b);
                b &= 0x7fffffffu;   // sin el bit de signo: |v[i]|
                if ((b & mask) == prefix)
                    local[(b >> shift) & (nb - 1)]++;
            }
            #pragma omp critical
            for (int j = 0; j < nb; j++)
                hist[j] += local[j];
        }

        int j = 0;
        while ((unsigned)k >= hist[j]) {
            k -= hist[j];
            j++;
        }
        prefix |= (unsigned)j << shift;
        mask |= (unsigned)(nb - 1) << shift;
    }

    float r;
    memcpy(&r, &prefix, sizeof r);
    return r;
}

float median_abs(const float *v, int n)
{
    return select_abs_kth(v, n, n / 2);
}
