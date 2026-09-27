#include <math.h>
#include <stdlib.h>
#include "dct.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef void (*dct1d_fn)(const float *, float *, int, const float *);

// tabla de cosenos cacheada: table[i*n+k] = cos(pi/n * (i+0.5) * k).
// Sirve para las dos direcciones (DCT-II y su inversa), solo cambia como se suma.
static float *cache_table = NULL;
static int cache_n = 0;

static const float *get_cos_table(int n)
{
    if (cache_n == n)
        return cache_table;

    free(cache_table);
    // alineada a 64 bytes: esta tabla es la que mas se relee en el programa
    // entero (una vez por cada multiplicacion del DCT, en el bucle caliente).
    if (posix_memalign((void **)&cache_table, 64, (size_t)n * n * sizeof(float)) != 0)
        cache_table = NULL;
    for (int i = 0; i < n; i++)
        for (int k = 0; k < n; k++)
            cache_table[i * n + k] = (float)cos(M_PI / n * (i + 0.5) * k);
    cache_n = n;
    return cache_table;
}

static void dct2_1d(const float *in, float *out, int n, const float *table)
{
    for (int k = 0; k < n; k++) {
        double sum = 0.0;
        for (int i = 0; i < n; i++)
            sum += in[i] * table[i * n + k];
        out[k] = (float)sum;
    }
}

// inversa exacta de dct2_1d
static void dct3_1d(const float *in, float *out, int n, const float *table)
{
    for (int i = 0; i < n; i++) {
        double sum = in[0] / n;
        for (int k = 1; k < n; k++)
            sum += (2.0 / n) * in[k] * table[i * n + k];
        out[i] = (float)sum;
    }
}

// cada fila es independiente de las demas, asi que se reparten entre hilos;
// cada uno necesita su propio escritorio (tmp) para no pisarse.
static void transform_rows(float *data, int nx, int ny, dct1d_fn f)
{
    const float *table = get_cos_table(nx);
    #pragma omp parallel if(ny > 32)
    {
        float *tmp = malloc(nx * sizeof(float));
        #pragma omp for
        for (int iy = 0; iy < ny; iy++) {
            float *row = data + (size_t)iy * nx;
            f(row, tmp, nx, table);
            for (int ix = 0; ix < nx; ix++)
                row[ix] = tmp[ix];
        }
        free(tmp);
    }
}

static void transform_cols(float *data, int nx, int ny, dct1d_fn f)
{
    const float *table = get_cos_table(ny);
    #pragma omp parallel if(nx > 32)
    {
        float *col_in = malloc(ny * sizeof(float));
        float *col_out = malloc(ny * sizeof(float));
        #pragma omp for
        for (int ix = 0; ix < nx; ix++) {
            for (int iy = 0; iy < ny; iy++)
                col_in[iy] = data[(size_t)iy * nx + ix];
            f(col_in, col_out, ny, table);
            for (int iy = 0; iy < ny; iy++)
                data[(size_t)iy * nx + ix] = col_out[iy];
        }
        free(col_in);
        free(col_out);
    }
}

void dct2d_forward(float *data, int nx, int ny)
{
    transform_rows(data, nx, ny, dct2_1d);
    transform_cols(data, nx, ny, dct2_1d);
}

void dct2d_inverse(float *data, int nx, int ny)
{
    transform_rows(data, nx, ny, dct3_1d);
    transform_cols(data, nx, ny, dct3_1d);
}
