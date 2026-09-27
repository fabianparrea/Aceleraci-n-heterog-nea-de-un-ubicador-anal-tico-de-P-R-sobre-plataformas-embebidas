#include <float.h>
#include <math.h>
#include <string.h>
#include "wirelength.h"

// dim 0 es x, dim 1 es y
static inline float pin_coord(const Pin *p, const float *v, int n, int dim)
{
    return dim ? v[n + p->cell] + p->dy : v[p->cell] + p->dx;
}

double compute_hpwl(const Netlist *nl, const float *v)
{
    int n = nl->num_cells;
    double total = 0.0;

    for (int i = 0; i < nl->num_nets; i++) {
        const Net *net = &nl->nets[i];
        if (net->degree < 2)
            continue;
        const Pin *pins = &nl->pins[net->pin_start];

        float xmin = FLT_MAX, xmax = -FLT_MAX;
        float ymin = FLT_MAX, ymax = -FLT_MAX;
        for (int k = 0; k < net->degree; k++) {
            float x = pin_coord(&pins[k], v, n, 0);
            float y = pin_coord(&pins[k], v, n, 1);
            if (x < xmin) xmin = x;
            if (x > xmax) xmax = x;
            if (y < ymin) ymin = y;
            if (y > ymax) ymax = y;
        }
        total += (xmax - xmin) + (ymax - ymin);
    }
    return total;
}

// WA de una red en una dimension. Los exponentes se restan al max/min para que no se desborde exp.
static double wa_net(const Netlist *nl, const Net *net, const float *v, int dim,
                     float inv_gamma, float *grad)
{
    int n = nl->num_cells;
    const Pin *pins = &nl->pins[net->pin_start];

    float lo = FLT_MAX, hi = -FLT_MAX;
    for (int k = 0; k < net->degree; k++) {
        float c = pin_coord(&pins[k], v, n, dim);
        if (c < lo) lo = c;
        if (c > hi) hi = c;
    }

    double sum_p = 0, wsum_p = 0, sum_m = 0, wsum_m = 0;
    for (int k = 0; k < net->degree; k++) {
        float c = pin_coord(&pins[k], v, n, dim);
        double ap = expf((c - hi) * inv_gamma);
        double am = expf((lo - c) * inv_gamma);
        sum_p += ap;
        wsum_p += c * ap;
        sum_m += am;
        wsum_m += c * am;
    }
    double wa_p = wsum_p / sum_p;
    double wa_m = wsum_m / sum_m;

    if (grad) {
        for (int k = 0; k < net->degree; k++) {
            float c = pin_coord(&pins[k], v, n, dim);
            double ap = expf((c - hi) * inv_gamma);
            double am = expf((lo - c) * inv_gamma);
            double g = ap / sum_p * (1 + (c - wa_p) * inv_gamma)
                     - am / sum_m * (1 - (c - wa_m) * inv_gamma);
            grad[dim * n + pins[k].cell] += g;
        }
    }
    return wa_p - wa_m;
}

double compute_wirelength(const Netlist *nl, const float *v, float gamma, float *grad)
{
    float inv_gamma = 1.0f / gamma;
    double total = 0.0;

    if (grad)
        memset(grad, 0, 2 * nl->num_cells * sizeof(float));

    for (int i = 0; i < nl->num_nets; i++) {
        const Net *net = &nl->nets[i];
        if (net->degree < 2)
            continue;
        total += wa_net(nl, net, v, 0, inv_gamma, grad);
        total += wa_net(nl, net, v, 1, inv_gamma, grad);
    }
    return total;
}
