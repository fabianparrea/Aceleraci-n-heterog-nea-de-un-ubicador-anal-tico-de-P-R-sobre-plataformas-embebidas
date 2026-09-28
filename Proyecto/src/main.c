#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bookshelf.h"
#include "wirelength.h"
#include "place.h"
#include "legalize.h"
#include "viz.h"

// nombre del benchmark sin carpeta ni ".aux", para nombrar los archivos de salida
static void base_name(const char *aux_path, char *out, size_t size)
{
    const char *slash = strrchr(aux_path, '/');
    const char *start = slash ? slash + 1 : aux_path;
    snprintf(out, size, "%s", start);
    char *dot = strrchr(out, '.');
    if (dot && strcmp(dot, ".aux") == 0)
        *dot = '\0';
}

// que porcentaje del die (el rectangulo que envuelve todas las celdas) ocupan
// las celdas puestas ahi adentro
static float utilization(const Netlist *nl, const float *v)
{
    int n = nl->num_cells;
    float xmin = FLT_MAX, xmax = -FLT_MAX, ymin = FLT_MAX, ymax = -FLT_MAX;
    double cell_area = 0.0;
    for (int i = 0; i < n; i++) {
        const Cell *c = &nl->cells[i];
        float x0 = v[i] - c->width / 2, x1 = v[i] + c->width / 2;
        float y0 = v[n + i] - c->height / 2, y1 = v[n + i] + c->height / 2;
        if (x0 < xmin) xmin = x0;
        if (x1 > xmax) xmax = x1;
        if (y0 < ymin) ymin = y0;
        if (y1 > ymax) ymax = y1;
        cell_area += c->area;
    }
    double die_area = (double)(xmax - xmin) * (ymax - ymin);
    return die_area > 0 ? (float)(100.0 * cell_area / die_area) : 0.0f;
}

int main(int argc, char **argv)
{
    // sin buffer: si stdout no es una terminal (por ejemplo, con tee a un
    // archivo) C junta la salida y no la escribe hasta que se llena o el
    // programa termina, y con corridas de minutos eso deja el progreso
    // invisible hasta el final.
    setvbuf(stdout, NULL, _IONBF, 0);

    if (argc < 2) {
        fprintf(stderr, "uso: %s benchmark.aux\n", argv[0]);
        return 1;
    }

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    Netlist nl;
    if (bookshelf_read(argv[1], &nl) < 0)
        return 1;
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double t_parseo = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) * 1e-9;

    float *v = malloc(2 * nl.num_cells * sizeof(float));
    if (!v) {
        netlist_free(&nl);
        return 1;
    }
    netlist_get_positions(&nl, v);

    printf("celdas: %d  redes: %d  pines: %d\n", nl.num_cells, nl.num_nets, nl.num_pins);
    printf("utilizacion: %.1f%%\n", utilization(&nl, v));
    printf("HPWL inicial: %.6e\n", compute_hpwl(&nl, v));

    // clock() mide CPU de todos los hilos sumada, no de reloj: con OpenMP da un
    // numero varias veces mayor al real. CLOCK_MONOTONIC si es tiempo de pared.
    clock_gettime(CLOCK_MONOTONIC, &t0);
    float overflow;
    PlaceTimings pt = {0};
    run_placement(&nl, &overflow, &pt);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double secs = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) * 1e-9;

    netlist_get_positions(&nl, v);
    printf("HPWL sin legalizar: %.6e  overflow: %.3f  tiempo: %.1f s\n",
           compute_hpwl(&nl, v), overflow, secs);

    clock_gettime(CLOCK_MONOTONIC, &t0);
    legalize(&nl, v);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double t_legalizacion = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) * 1e-9;
    netlist_set_positions(&nl, v);
    printf("HPWL legalizado: %.6e\n", compute_hpwl(&nl, v));

    char base[256];
    base_name(argv[1], base, sizeof base);
    char path[300];

    clock_gettime(CLOCK_MONOTONIC, &t0);
    snprintf(path, sizeof path, "%s.out.pl", base);
    if (write_pl(path, &nl, v) == 0)
        printf("posiciones finales: %s\n", path);

    snprintf(path, sizeof path, "%s.ppm", base);
    write_layout_ppm(path, &nl, v, 1000);
    printf("imagen del layout: %s\n", path);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double t_escritura = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) * 1e-9;

    // desglose por etapa, para perfilado por instrumentacion (una corrida,
    // sin repetir 100 veces: los benchmarks reales tardan minutos cada uno)
    printf("tiempos por etapa (s): parseo=%.4f  initial_place=%.4f  "
           "nesterov=%.4f  legalizacion=%.4f  escritura=%.4f\n",
           t_parseo, pt.t_initial_place, pt.t_nesterov, t_legalizacion, t_escritura);

    free(v);
    netlist_free(&nl);
    return 0;
}
