#include <stdlib.h>
#include "initial_place.h"

// aplica el Laplaciano de conectividad (cada red conecta sus celdas con un
// resorte de peso k/(k-1)) a "vec" en una sola dimension. Conectar cada par
// de una red de k pines costaria O(k^2); restando el promedio de la red da
// exactamente el mismo resultado en O(k) (es una identidad algebraica del
// modelo de resorte tipo "estrella").
//
// con_fijas=1: usa el valor real de vec[] en las celdas fijas (para sacar el
// residuo inicial, que si depende de donde estan). con_fijas=0: las trata
// como si valieran 0 (para aplicar el operador a una direccion de busqueda,
// que no tiene "posicion" en las celdas fijas).
static void apply_laplacian(const Netlist *nl, const float *vec, int con_fijas, float *out)
{
    int n = nl->num_cells;
    for (int i = 0; i < n; i++)
        out[i] = 0.0f;

    for (int e = 0; e < nl->num_nets; e++) {
        const Net *net = &nl->nets[e];
        int k = net->degree;
        if (k < 2)
            continue;
        const Pin *pins = &nl->pins[net->pin_start];

        double sum = 0.0;
        for (int p = 0; p < k; p++) {
            int cell = pins[p].cell;
            sum += (nl->cells[cell].fixed && !con_fijas) ? 0.0 : vec[cell];
        }
        double mean = sum / k;
        double coef = (double)k / (k - 1);

        for (int p = 0; p < k; p++) {
            int cell = pins[p].cell;
            if (nl->cells[cell].fixed)
                continue;
            out[cell] += (float)(coef * (vec[cell] - mean));
        }
    }
}

// gradiente conjugado para A*vec = b (A y b nunca se arman: todo sale de
// apply_laplacian). Solo mueve las celdas movibles, las fijas se quedan con
// el valor que ya traian en vec.
static void solve_cg(const Netlist *nl, float *vec, int max_iters)
{
    int n = nl->num_cells;
    float *r = malloc(n * sizeof(float));
    float *p = malloc(n * sizeof(float));
    float *ap = malloc(n * sizeof(float));

    apply_laplacian(nl, vec, 1, r);
    for (int i = 0; i < n; i++)
        r[i] = nl->cells[i].fixed ? 0.0f : -r[i];
    for (int i = 0; i < n; i++)
        p[i] = r[i];

    double rs_old = 0.0;
    for (int i = 0; i < n; i++)
        rs_old += (double)r[i] * r[i];

    for (int iter = 0; iter < max_iters && rs_old > 1e-9; iter++) {
        apply_laplacian(nl, p, 0, ap);
        double pap = 0.0;
        for (int i = 0; i < n; i++)
            pap += (double)p[i] * ap[i];
        if (pap < 1e-12)
            break;
        double alpha = rs_old / pap;

        for (int i = 0; i < n; i++) {
            if (nl->cells[i].fixed)
                continue;
            vec[i] += (float)(alpha * p[i]);
            r[i] -= (float)(alpha * ap[i]);
        }
        double rs_new = 0.0;
        for (int i = 0; i < n; i++)
            rs_new += (double)r[i] * r[i];
        if (rs_new < 1e-9)
            break;
        double beta = rs_new / rs_old;
        for (int i = 0; i < n; i++)
            if (!nl->cells[i].fixed)
                p[i] = r[i] + (float)beta * p[i];
        rs_old = rs_new;
    }

    free(r);
    free(p);
    free(ap);
}

void initial_place(const Netlist *nl, float *v)
{
    int n = nl->num_cells;
    float *x = malloc(n * sizeof(float));
    float *y = malloc(n * sizeof(float));
    for (int i = 0; i < n; i++) {
        x[i] = v[i];
        y[i] = v[n + i];
    }

    int max_iters = n < 100 ? n : 100;
    solve_cg(nl, x, max_iters);
    solve_cg(nl, y, max_iters);

    for (int i = 0; i < n; i++) {
        if (nl->cells[i].fixed)
            continue;
        v[i] = x[i];
        v[n + i] = y[i];
    }

    free(x);
    free(y);
}
