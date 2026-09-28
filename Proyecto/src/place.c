#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "place.h"
#include "wirelength.h"
#include "density.h"
#include "optimizer.h"
#include "grid.h"
#include "initial_place.h"

#define TARGET_OVERFLOW 0.1f
#define MIN_ITERS 800
#define MAX_ITERS 1000
#define MAX_STEP_GROWTH 1.3f
#define OVERFLOW_PENALTY 5.0

// combina wirelength + lambda*densidad, y deja en cero las celdas fijas: a
// wirelength no le importa si una celda es un pin fijo, le calcula gradiente
// igual, pero esas celdas no se tienen que mover.
static void combine_grad(const Netlist *nl, float *grad, const float *grad_w,
                         const float *grad_d, float lambda)
{
    int n = nl->num_cells;
    for (int i = 0; i < n; i++) {
        float gx = grad_w[i] + lambda * grad_d[i];
        float gy = grad_w[n + i] + lambda * grad_d[n + i];
        int fixed = nl->cells[i].fixed;
        grad[i] = fixed ? 0.0f : gx;
        grad[n + i] = fixed ? 0.0f : gy;
    }
}

static float norm_diff(const float *a, const float *b, int n)
{
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        double d = (double)a[i] - b[i];
        sum += d * d;
    }
    return (float)sqrt(sum);
}

// step tipo secante (Barzilai-Borwein): que tanto cambio el gradiente entre los
// dos ultimos puntos nos dice que tan grande puede ser el siguiente paso. Se
// limita cuanto puede crecer de una iteracion a la otra (puede bajar libre)
// porque sin eso, cuando el gradiente casi no cambia, el paso se dispara y el
// loop diverge.
static float estimate_step(const float *v, const float *v_prev,
                           const float *g, const float *g_prev, int n2,
                           float max_step, float prev_step)
{
    float dv = norm_diff(v, v_prev, n2);
    float dg = norm_diff(g, g_prev, n2);
    float step = dg > 1e-9f ? dv / dg : max_step;
    if (step > max_step) step = max_step;
    if (step > prev_step * MAX_STEP_GROWTH) step = prev_step * MAX_STEP_GROWTH;
    if (step < 1e-6f) step = 1e-6f;
    return step;
}

// segundos entre dos marcas de CLOCK_MONOTONIC (tiempo de pared, no de CPU)
static double elapsed_seconds(struct timespec t0, struct timespec t1)
{
    return (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) * 1e-9;
}

static int cmp_float(const void *a, const void *b)
{
    float fa = *(const float *)a, fb = *(const float *)b;
    return (fa > fb) - (fa < fb);
}

// la mediana de un arreglo de magnitudes, usando abs[] como escritorio (se
// reordena, no importa porque el que llama ya tiene su propia copia sin tocar)
static float median_abs(float *abs_vals, const float *v, int n2)
{
    for (int i = 0; i < n2; i++)
        abs_vals[i] = fabsf(v[i]);
    qsort(abs_vals, n2, sizeof(float), cmp_float);
    return abs_vals[n2 / 2];
}

// calibracion inicial de lambda: cuanto mayor es el gradiente "tipico" de
// wirelength comparado con el de densidad, en el punto de arranque. Se usa
// la mediana y no la suma porque unas pocas celdas (las que quedan justo en
// el borde de un macro grande, por ejemplo) pueden tener un gradiente de
// densidad enorme y arruinar un promedio.
static float calibrate_lambda(float *scratch, const float *grad_w, const float *grad_d, int n2)
{
    float med_w = median_abs(scratch, grad_w, n2);
    float med_d = median_abs(scratch, grad_d, n2);
    return med_d > 1e-12f ? med_w / med_d : 1.0f;
}

void run_placement(Netlist *nl, float *out_overflow, PlaceTimings *out_timings)
{
    struct timespec t0, t1;

    int n = nl->num_cells;
    int n2 = 2 * n;

    Grid grid;
    grid_init(&grid, nl);
    float avg_bin = 0.5f * (grid.bin_w + grid.bin_h);
    float gamma_min = avg_bin;
    float gamma_max = 20.0f * avg_bin;
    float max_step = 4.0f * avg_bin;

    float *grad = malloc(n2 * sizeof(float));
    float *grad_w = malloc(n2 * sizeof(float));
    float *grad_d = malloc(n2 * sizeof(float));
    float *scratch = malloc(n2 * sizeof(float));

    NesterovState st;
    st.n2 = n2;
    st.u = malloc(n2 * sizeof(float));
    st.v = malloc(n2 * sizeof(float));
    st.v_prev = malloc(n2 * sizeof(float));
    st.grad_prev = malloc(n2 * sizeof(float));
    st.a = 1.0f;
    st.lambda = 0.0f;

    // arranque real: un colocado por minimos cuadrados (cada red tira de sus
    // celdas como un resorte) reparte las celdas segun conectividad antes de
    // que entre densidad, en vez de dejarlas donde las trae el .pl (que en
    // estos benchmarks suele venir casi todo amontonado en un punto).
    netlist_get_positions(nl, st.v);
    clock_gettime(CLOCK_MONOTONIC, &t0);
    initial_place(nl, st.v);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    if (out_timings)
        out_timings->t_initial_place = elapsed_seconds(t0, t1);
    memcpy(st.u, st.v, n2 * sizeof(float));
    float gamma = gamma_max;
    float overflow = 1.0f;

    // arranque: se calibra lambda y se arma un punto "anterior" artificial
    // (perturbando en direccion del gradiente) para poder estimar el primer
    // paso con la formula tipo secante de mas abajo.
    compute_wirelength(nl, st.v, gamma, grad_w);
    compute_density(nl, &grid, st.v, grad_d, &overflow);
    float lambda = calibrate_lambda(scratch, grad_w, grad_d, n2);
    // una vez que la densidad ya no hace falta (el layout esta parejo) su
    // gradiente se acerca a cero y la proporcion se dispara sin control; por
    // eso lambda se limita a dos ordenes de magnitud alrededor de esta primera
    // calibracion (el rastreo del mejor punto de mas abajo es la red de
    // seguridad real contra una mala racha; esto es solo para no desbordar).
    float lambda_min = lambda * 0.01f, lambda_max = lambda * 100.0f;
    st.lambda = lambda;
    combine_grad(nl, grad, grad_w, grad_d, lambda);

    for (int i = 0; i < n2; i++)
        st.v_prev[i] = st.v[i] + 100.0f * grad[i];
    compute_wirelength(nl, st.v_prev, gamma, grad_w);
    compute_density(nl, &grid, st.v_prev, grad_d, &overflow);
    combine_grad(nl, st.grad_prev, grad_w, grad_d, lambda);

    st.step = estimate_step(st.v, st.v_prev, grad, st.grad_prev, n2, max_step, max_step);

    // Nesterov no garantiza que el HPWL baje en cada iteracion (menos con un
    // macro grande metido en medio, que puede desestabilizar el paso): en vez
    // de quedarnos con la ultima posicion, guardamos la que tenga mejor HPWL
    // "penalizado", igual que el sHPWL con el que RePlAce mide sus resultados:
    // en vez de descalificar de una un overflow por encima del objetivo, lo
    // encarece proporcionalmente, asi no se pierde un buen punto que quedo
    // justo antes de cruzar el umbral.
    float *best_u = malloc(n2 * sizeof(float));
    double best_score = DBL_MAX;
    float best_overflow = overflow;

    // en algunos circuitos la mediana de gradientes no refleja que el overflow
    // sigue muy alto (wirelength se acomoda mas rapido de lo que densidad
    // logra resolver, y la proporcion medida hasta baja en vez de subir): si
    // el overflow casi no mejora en una ventana larga, se fuerza mas peso a
    // densidad multiplicando lambda, en vez de confiar solo en esa proporcion
    // cruda. La ventana es de 300 iteraciones (no 100) porque la mejora normal
    // de este metodo es despareja -- adaptec1 mejora poco al principio y
    // acelera mucho despues -- y con una ventana corta se confunde ese arranque
    // lento con un atasco real y se sobre-escala un circuito que iba bien.
    float escalate = 1.0f;
    float overflow_ref = overflow;
    const int ESCALATE_WINDOW = 300;

    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (int iter = 0; iter < MAX_ITERS; iter++) {
        compute_wirelength(nl, st.v, gamma, grad_w);
        compute_density(nl, &grid, st.v, grad_d, &overflow);
        float raw_lambda = calibrate_lambda(scratch, grad_w, grad_d, n2);
        if (raw_lambda < lambda_min) raw_lambda = lambda_min;
        if (raw_lambda > lambda_max) raw_lambda = lambda_max;
        // promedio movil en vez de tomar la calibracion cruda: wirelength y
        // densidad se empujan entre si (mas densidad mueve celdas, eso cambia
        // el gradiente de wirelength, eso cambia la siguiente calibracion), y
        // sin suavizar ese lazo puede oscilar en vez de converger.
        lambda = 0.9f * lambda + 0.1f * (raw_lambda * escalate);
        st.lambda = lambda;
        combine_grad(nl, grad, grad_w, grad_d, lambda);
        double hpwl = compute_hpwl(nl, st.v);

        if (iter % 100 == 0)
            printf("  iter %4d: HPWL %.4e  overflow %.3f\n", iter, hpwl, overflow);

        if (iter > 0 && iter % ESCALATE_WINDOW == 0) {
            if (overflow > overflow_ref * 0.97f && escalate < 30.0f)
                escalate *= 1.5f;
            overflow_ref = overflow;
        }

        double excess = overflow > TARGET_OVERFLOW ? overflow - TARGET_OVERFLOW : 0.0;
        double score = hpwl * (1.0 + OVERFLOW_PENALTY * excess);
        if (score < best_score) {
            best_score = score;
            best_overflow = overflow;
            memcpy(best_u, st.u, n2 * sizeof(float));
        }
        if (iter > MIN_ITERS && overflow <= TARGET_OVERFLOW)
            break;

        // gamma arranca ancho (overflow alto) y se va cerrando conforme se ordena
        float t = overflow < 0.0f ? 0.0f : (overflow > 1.0f ? 1.0f : overflow);
        gamma = gamma_min + (gamma_max - gamma_min) * t;

        st.step = estimate_step(st.v, st.v_prev, grad, st.grad_prev, n2, max_step, st.step);
        nesterov_step(&st, grad);
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    if (out_timings)
        out_timings->t_nesterov = elapsed_seconds(t0, t1);

    netlist_set_positions(nl, best_score < DBL_MAX ? best_u : st.u);
    if (out_overflow)
        *out_overflow = best_overflow;

    free(best_u);
    free(grad);
    free(grad_w);
    free(grad_d);
    free(scratch);
    free(st.u);
    free(st.v);
    free(st.v_prev);
    free(st.grad_prev);
    grid_free(&grid);
}
