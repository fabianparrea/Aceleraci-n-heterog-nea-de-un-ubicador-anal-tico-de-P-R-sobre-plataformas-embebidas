#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "prof.h"

static const char *stage_names[PROF_NUM_STAGES] = {
    "wirelength", "density", "lambda", "hpwl", "update",
    "splat", "poisson", "field", "density_grad",
    "wl_kernel", "h2d", "d2h",
};

// cada fila guarda primero el total de la iteracion y despues cada etapa
#define ROW_LEN (PROF_NUM_STAGES + 1)

static double *rows = NULL;
static int num_rows = 0, cap_rows = 0;
static int row_open = 0;
static double row_start;

double prof_now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

void prof_begin_iter(void)
{
    if (num_rows == cap_rows) {
        int cap = cap_rows ? 2 * cap_rows : 1024;
        double *p = realloc(rows, (size_t)cap * ROW_LEN * sizeof(double));
        if (!p)
            return;   // sin memoria se pierde la muestra, no la corrida
        rows = p;
        cap_rows = cap;
    }
    double *row = rows + (size_t)num_rows * ROW_LEN;
    for (int s = 0; s < ROW_LEN; s++)
        row[s] = 0.0;
    row_open = 1;
    row_start = prof_now();
}

void prof_add(ProfStage stage, double seconds)
{
    if (row_open)
        rows[(size_t)num_rows * ROW_LEN + 1 + stage] += seconds;
}

double prof_lap(ProfStage stage, double t0)
{
    double t1 = prof_now();
    prof_add(stage, t1 - t0);
    return t1;
}

void prof_end_iter(void)
{
    if (!row_open)
        return;
    rows[(size_t)num_rows * ROW_LEN] = prof_now() - row_start;
    num_rows++;
    row_open = 0;
}

int prof_write_csv(const char *path)
{
    FILE *f = fopen(path, "w");
    if (!f)
        return -1;
    fprintf(f, "iter,total");
    for (int s = 0; s < PROF_NUM_STAGES; s++)
        fprintf(f, ",%s", stage_names[s]);
    fprintf(f, "\n");
    for (int r = 0; r < num_rows; r++) {
        const double *row = rows + (size_t)r * ROW_LEN;
        fprintf(f, "%d", r);
        for (int s = 0; s < ROW_LEN; s++)
            fprintf(f, ",%.4f", row[s] * 1e3);
        fprintf(f, "\n");
    }
    fclose(f);
    return 0;
}

void prof_reset(void)
{
    free(rows);
    rows = NULL;
    num_rows = cap_rows = 0;
    row_open = 0;
}
