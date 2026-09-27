#include <math.h>
#include <stdlib.h>
#include "legalize.h"

typedef struct { float lo, hi; } Interval;

static int cmp_interval(const void *a, const void *b)
{
    float la = ((const Interval *)a)->lo, lb = ((const Interval *)b)->lo;
    return (la > lb) - (la < lb);
}

// los tramos de esta fila que ya ocupa una celda fija, ordenados y sin traslape
// entre si (una fija puede cruzar varias filas, por eso se revisa cada vez)
static int blocked_intervals(const Netlist *nl, const float *v, const Row *row, Interval *out)
{
    int n = nl->num_cells;
    int count = 0;
    float row_lo = row->y0, row_hi = row->y0 + row->height;

    for (int i = 0; i < n; i++) {
        if (!nl->cells[i].fixed)
            continue;
        const Cell *c = &nl->cells[i];
        float cy = v[n + i];
        if (cy + c->height / 2 <= row_lo || cy - c->height / 2 >= row_hi)
            continue;
        float cx = v[i];
        out[count].lo = cx - c->width / 2;
        out[count].hi = cx + c->width / 2;
        count++;
    }
    qsort(out, count, sizeof(Interval), cmp_interval);

    int m = 0;
    for (int i = 0; i < count; i++) {
        if (m > 0 && out[i].lo <= out[m - 1].hi) {
            if (out[i].hi > out[m - 1].hi)
                out[m - 1].hi = out[i].hi;
        } else {
            out[m++] = out[i];
        }
    }
    return m;
}

// empuja x justo despues de cualquier tramo bloqueado que [x, x+width) toque
static float skip_blocked(float x, float width, const Interval *blocked, int nb)
{
    for (int i = 0; i < nb; i++)
        if (x < blocked[i].hi && x + width > blocked[i].lo)
            x = blocked[i].hi;
    return x;
}

// insertion sort: las filas suelen tener pocas celdas, no hace falta nada mas rapido
static void sort_by_x(int *ids, int count, const float *v)
{
    for (int i = 1; i < count; i++) {
        int key = ids[i];
        float kx = v[key];
        int j = i - 1;
        while (j >= 0 && v[ids[j]] > kx) {
            ids[j + 1] = ids[j];
            j--;
        }
        ids[j + 1] = key;
    }
}

// un grupo de celdas contiguas que ya quedaron pegadas unas a otras (porque
// dejarlas en su lugar preferido se hubiera traslapado). sum_adj guarda la
// suma de "target menos el ancho acumulado antes de la celda dentro del
// grupo": dividido entre count da la posicion optima de la primera celda del
// grupo (el algoritmo de Abacus, resuelve el minimo de la suma de
// desplazamientos al cuadrado en forma cerrada).
typedef struct {
    int first, count;
    double sum_adj, width, x;
} Cluster;

// barre la fila de izquierda a derecha formando grupos: cada celda nueva se
// funde con el grupo anterior si se traslaparian, y el grupo fusionado se
// recoloca en su optimo comun. A diferencia de un barrido simple (que solo
// evita el traslape), esto minimiza cuanto se mueve cada celda en total.
static void legalize_row(Netlist *nl, float *v, const Row *row, const int *ids, int count,
                         Interval *blocked, Cluster *clusters)
{
    int nb = blocked_intervals(nl, v, row, blocked);
    int nc = 0;

    for (int k = 0; k < count; k++) {
        int id = ids[k];
        float width = nl->cells[id].width;
        float target = v[id] - width / 2;
        if (target < row->x0)
            target = row->x0;

        target = skip_blocked(target, width, blocked, nb);

        Cluster c = { k, 1, target, width, target };
        while (nc > 0 && c.x < clusters[nc - 1].x + clusters[nc - 1].width) {
            Cluster *prev = &clusters[nc - 1];
            c.sum_adj = prev->sum_adj + (c.sum_adj - prev->width * c.count);
            c.width += prev->width;
            c.count += prev->count;
            c.first = prev->first;
            c.x = c.sum_adj / c.count;
            if (c.x < row->x0)
                c.x = row->x0;
            nc--;
        }
        clusters[nc++] = c;
    }

    // las celdas dentro de un grupo quedan pegadas exactamente por su ancho
    // (ya redondeado a sitio en el arranque del grupo), sin volver a redondear
    // una por una -- eso podria correr una un pelo hacia la otra y traslaparlas.
    for (int ci = 0; ci < nc; ci++) {
        float sites = (float)((clusters[ci].x - row->x0) / row->site_w);
        float x = row->x0 + roundf(sites) * row->site_w;
        for (int k = clusters[ci].first; k < clusters[ci].first + clusters[ci].count; k++) {
            int id = ids[k];
            float width = nl->cells[id].width;
            v[id] = x + width / 2;
            x += width;
        }
    }

    // saneamiento final, barato: si fusionar dos grupos corrio el resultado
    // justo encima de un tramo bloqueado (posible, aunque cada celda ya
    // esquivo lo bloqueado por su cuenta al entrar), un ultimo barrido de
    // izquierda a derecha lo corrige. En el caso normal esto no mueve nada.
    float cursor = row->x0;
    for (int k = 0; k < count; k++) {
        int id = ids[k];
        float width = nl->cells[id].width;
        float x0 = v[id] - width / 2;
        if (x0 < cursor)
            x0 = cursor;
        x0 = skip_blocked(x0, width, blocked, nb);
        v[id] = x0 + width / 2;
        v[nl->num_cells + id] = row->y0 + row->height / 2;
        cursor = x0 + width;
    }
}

// filas ordenadas de abajo a arriba (el .scl no promete ningun orden)
static void sort_rows_by_y(int *order, int nr, const Row *rows)
{
    for (int i = 1; i < nr; i++) {
        int key = order[i];
        float ky = rows[key].y0;
        int j = i - 1;
        while (j >= 0 && rows[order[j]].y0 > ky) {
            order[j + 1] = order[j];
            j--;
        }
        order[j + 1] = key;
    }
}

typedef struct { int id; float x; } SpillEntry;

static int cmp_spill_desc(const void *a, const void *b)
{
    float xa = ((const SpillEntry *)a)->x, xb = ((const SpillEntry *)b)->x;
    return (xa < xb) - (xa > xb);
}

// si a una fila le tocaron mas celdas de las que caben, pasa el sobrante (las
// que quedan mas a la derecha) a la fila de arriba, en cadena si hace falta.
// Es preferible mover una celda a la fila vecina (un salto chico en y) que
// dejarla estirar el barrido de su fila hasta muy lejos en x.
static void spill_overflow(const Netlist *nl, const float *v, int *row_of,
                           const int *row_order, double *demand, SpillEntry *tmp)
{
    int n = nl->num_cells;
    for (int oi = 0; oi + 1 < nl->num_rows; oi++) {
        int r = row_order[oi];
        double cap = (double)nl->rows[r].num_sites * nl->rows[r].site_w;
        if (demand[r] <= cap)
            continue;
        int next = row_order[oi + 1];

        int count = 0;
        for (int i = 0; i < n; i++)
            if (row_of[i] == r) {
                tmp[count].id = i;
                tmp[count].x = v[i];
                count++;
            }
        // orden descendente por x: con overflow alto una fila puede juntar
        // miles de celdas, y ahi un insertion sort (O(count^2)) se nota mucho;
        // qsort es O(count log count).
        qsort(tmp, count, sizeof(SpillEntry), cmp_spill_desc);
        for (int k = 0; k < count && demand[r] > cap; k++) {
            int id = tmp[k].id;
            double w = nl->cells[id].width;
            row_of[id] = next;
            demand[r] -= w;
            demand[next] += w;
        }
    }
}

void legalize(Netlist *nl, float *v)
{
    if (nl->num_rows == 0)
        return;

    int n = nl->num_cells, nr = nl->num_rows;
    int *row_of = malloc(n * sizeof(int));
    int *ids = malloc(n * sizeof(int));
    SpillEntry *spill_tmp = malloc(n * sizeof(SpillEntry));
    Interval *blocked = malloc(n * sizeof(Interval));
    Cluster *clusters = malloc(n * sizeof(Cluster));
    double *demand = calloc(nr, sizeof(double));
    int *row_order = malloc(nr * sizeof(int));
    for (int r = 0; r < nr; r++)
        row_order[r] = r;
    sort_rows_by_y(row_order, nr, nl->rows);

    // a cada celda movible le toca la fila cuyo centro en y le queda mas cerca
    for (int i = 0; i < n; i++) {
        if (nl->cells[i].fixed) {
            row_of[i] = -1;
            continue;
        }
        float cy = v[n + i];
        int best = 0;
        float best_d = fabsf(cy - (nl->rows[0].y0 + nl->rows[0].height / 2));
        for (int r = 1; r < nr; r++) {
            float d = fabsf(cy - (nl->rows[r].y0 + nl->rows[r].height / 2));
            if (d < best_d) {
                best_d = d;
                best = r;
            }
        }
        row_of[i] = best;
        demand[best] += nl->cells[i].width;
    }

    spill_overflow(nl, v, row_of, row_order, demand, spill_tmp);

    for (int r = 0; r < nr; r++) {
        int count = 0;
        for (int i = 0; i < n; i++)
            if (row_of[i] == r)
                ids[count++] = i;
        sort_by_x(ids, count, v);
        legalize_row(nl, v, &nl->rows[r], ids, count, blocked, clusters);
    }

    free(row_order);
    free(demand);
    free(clusters);
    free(blocked);
    free(spill_tmp);
    free(ids);
    free(row_of);
}
