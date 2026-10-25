#ifndef PROF_H
#define PROF_H

// tambien se usa desde los .cu de la version CUDA (que se compilan como C++)
#ifdef __cplusplus
extern "C" {
#endif

// Perfilado por instrumentacion del ciclo de Nesterov: cada iteracion deja una
// muestra de tiempo de pared por etapa (una fila del CSV por iteracion). Fuera
// de una iteracion abierta prof_add no hace nada, asi las llamadas de arranque
// (antes del ciclo) y las pruebas no se mezclan con las muestras.
typedef enum {
    PROF_WIRELENGTH,
    PROF_DENSITY,
    PROF_LAMBDA,
    PROF_HPWL,
    PROF_UPDATE,
    // partes de la densidad (ya estan sumadas dentro de PROF_DENSITY)
    PROF_SPLAT,
    PROF_POISSON,
    PROF_FIELD,
    PROF_DENSITY_GRAD,
    // solo en la version CUDA (ya sumadas dentro de wirelength y densidad)
    PROF_WL_KERNEL,
    PROF_H2D,
    PROF_D2H,
    PROF_NUM_STAGES
} ProfStage;

// segundos de CLOCK_MONOTONIC (tiempo de pared, no de CPU)
double prof_now(void);

void prof_begin_iter(void);
void prof_add(ProfStage stage, double seconds);
// suma a la etapa el tiempo desde t0 y devuelve el instante actual, para
// encadenar una etapa detras de otra sin repetir prof_now
double prof_lap(ProfStage stage, double t0);
void prof_end_iter(void);

// una fila por iteracion, tiempos en milisegundos. Devuelve 0 si pudo escribir.
int prof_write_csv(const char *path);
void prof_reset(void);

#ifdef __cplusplus
}
#endif

#endif
