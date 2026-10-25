#include <math.h>
#include <stdlib.h>
#include "check.h"
#include "netlist.h"
#include "grid.h"
#include "wirelength.h"
#include "density.h"
#include "dct.h"
#include "gpu.h"

// La version GPU tiene que dar lo mismo que la de CPU salvo redondeo (float en
// vez de double en algunas sumas, otro orden en los atomicos y la DCT por FFT).
// Se compara el error relativo en norma 2 de todo el vector; un error de
// indices daria algo del orden de 1, muy lejos de las tolerancias.

static double rel_err(const float *a, const float *ref, int n)
{
    double num = 0.0, den = 0.0;
    for (int i = 0; i < n; i++) {
        double d = (double)a[i] - ref[i];
        num += d * d;
        den += (double)ref[i] * ref[i];
    }
    return den > 0 ? sqrt(num / den) : sqrt(num);
}

static float frand(void)
{
    return rand() / (float)RAND_MAX;
}

static void test_dct(void)
{
    int sizes[] = {16, 64, 256, 512};
    for (int s = 0; s < 4; s++) {
        int n = sizes[s];
        float *cpu = malloc((size_t)n * n * sizeof(float));
        float *gpu = malloc((size_t)n * n * sizeof(float));
        srand(n);
        for (int i = 0; i < n * n; i++)
            cpu[i] = gpu[i] = frand();

        dct2d_forward(cpu, n, n);
        CHECK(gpu_dct2d(gpu, n, 0) == 0);
        double e_fwd = rel_err(gpu, cpu, n * n);

        // la vuelta parte de la misma entrada en las dos (la ida de la CPU)
        for (int i = 0; i < n * n; i++)
            gpu[i] = cpu[i];
        dct2d_inverse(cpu, n, n);
        CHECK(gpu_dct2d(gpu, n, 1) == 0);
        double e_inv = rel_err(gpu, cpu, n * n);

        printf("  dct %3dx%-3d  error ida %.1e  vuelta %.1e\n", n, n, e_fwd, e_inv);
        CHECK(e_fwd < 1e-4);
        CHECK(e_inv < 1e-4);
        free(cpu);
        free(gpu);
    }
}

// netlist sintetico del tamano de adaptec1 (sale la misma grilla de 256x256):
// celdas de varios tamanos, una de cada cien fija y grande como un macro, la
// mayoria de las redes de 2 a 4 pines como en los benchmarks (algunas de hasta
// 30, otras de 1 que se saltan, y una de 3000), y la mitad de las celdas
// amontonadas en el centro para que haya overflow
static void make_netlist(Netlist *nl, int num_cells, int num_nets)
{
    srand(1234);
    nl->num_cells = num_cells;
    nl->num_nets = num_nets;
    nl->cells = calloc(num_cells, sizeof(Cell));
    nl->nets = calloc(num_nets, sizeof(Net));
    nl->num_rows = 0;
    nl->rows = NULL;

    float die = 2000.0f;
    for (int i = 0; i < num_cells; i++) {
        Cell *c = &nl->cells[i];
        c->id = i;
        c->fixed = i % 100 == 0;
        c->width = c->fixed ? 40 + 60 * frand() : 1 + 7 * frand();
        c->height = c->fixed ? 40 + 60 * frand() : 2.0f;
        c->area = c->width * c->height;
        float span = i % 2 ? die : 0.2f * die;
        float start = i % 2 ? 0.0f : 0.4f * die;
        c->x = start + span * frand();
        c->y = start + span * frand();
    }

    int cap = num_nets * 8, num_pins = 0;
    nl->pins = malloc(cap * sizeof(Pin));
    for (int e = 0; e < num_nets; e++) {
        int deg;
        if (e == 0)
            deg = 3000;
        else if (e % 50 == 1)
            deg = 1;
        else if (rand() % 5)
            deg = 2 + rand() % 3;
        else
            deg = 5 + rand() % 26;
        if (num_pins + deg > cap) {
            cap = 2 * (num_pins + deg);
            nl->pins = realloc(nl->pins, cap * sizeof(Pin));
        }
        Net *net = &nl->nets[e];
        net->id = e;
        net->pin_start = num_pins;
        net->degree = deg;
        for (int k = 0; k < deg; k++) {
            Pin *p = &nl->pins[num_pins++];
            p->cell = rand() % num_cells;
            p->net = e;
            const Cell *c = &nl->cells[p->cell];
            p->dx = (frand() - 0.5f) * c->width;
            p->dy = (frand() - 0.5f) * c->height;
        }
    }
    nl->num_pins = num_pins;
}

static void test_wirelength(const Netlist *nl, const Grid *grid, GpuPlacer *g, const float *v)
{
    int n2 = 2 * nl->num_cells;
    float *gc = malloc(n2 * sizeof(float));
    float *gg = malloc(n2 * sizeof(float));

    // gamma chico y grande, como al final y al principio del ciclo
    float gammas[2] = {grid->bin_w, 20.0f * grid->bin_w};
    for (int t = 0; t < 2; t++) {
        double wc = compute_wirelength(nl, v, gammas[t], gc);
        double wg = gpu_wirelength(g, v, gammas[t], gg);
        double e_total = fabs(wg - wc) / wc;
        double e_grad = rel_err(gg, gc, n2);
        printf("  wirelength gamma=%-7.1f error total %.1e  gradiente %.1e\n",
               gammas[t], e_total, e_grad);
        CHECK(e_total < 1e-5);
        CHECK(e_grad < 1e-4);
    }
    // sin gradiente tambien tiene que dar el total
    double wc = compute_wirelength(nl, v, gammas[0], NULL);
    CHECK_NEAR(gpu_wirelength(g, v, gammas[0], NULL) / wc, 1.0, 1e-5);

    // una de cada cien celdas muy lejos del resto, como las que manda afuera un
    // mal paso del optimizador: en float, el lado + del gradiente no puede
    // perder precision por un pin lejano en el otro extremo de la red
    int n = nl->num_cells;
    float *far = malloc(n2 * sizeof(float));
    for (int i = 0; i < n2; i++)
        far[i] = v[i];
    for (int i = 0; i < n; i += 100) {
        far[i] = -1e6f;
        far[n + i] = -1e6f;
    }
    compute_wirelength(nl, far, gammas[0], gc);
    gpu_wirelength(g, far, gammas[0], gg);
    double e_far = rel_err(gg, gc, n2);
    printf("  wirelength con celdas lejos         gradiente %.1e\n", e_far);
    CHECK(e_far < 1e-4);
    free(far);

    free(gc);
    free(gg);
}

static void test_density(const Netlist *nl, Grid *grid, GpuPlacer *g, const float *v)
{
    int n2 = 2 * nl->num_cells;
    float *gc = malloc(n2 * sizeof(float));
    float *gg = malloc(n2 * sizeof(float));

    float oc, og;
    double ec = compute_density(nl, grid, v, gc, &oc);
    double eg = gpu_density(g, v, gg, &og);
    double e_energy = fabs(eg - ec) / fabs(ec);
    double e_grad = rel_err(gg, gc, n2);
    printf("  densidad   overflow cpu %.6f gpu %.6f  error energia %.1e  gradiente %.1e\n",
           oc, og, e_energy, e_grad);
    CHECK(oc > 0.0f);   // si no hay overflow la prueba no estaria ejercitando nada
    CHECK(fabs(og - oc) < 1e-4);
    CHECK(e_energy < 1e-4);
    CHECK(e_grad < 1e-3);

    // sin gradiente el overflow tiene que salir igual
    float o_nograd;
    gpu_density(g, v, NULL, &o_nograd);
    CHECK_NEAR(o_nograd, og, 1e-6);

    free(gc);
    free(gg);
}

int main(void)
{
    test_dct();

    Netlist nl;
    make_netlist(&nl, 200000, 210000);
    Grid grid;
    grid_init(&grid, &nl);
    printf("  netlist sintetico: %d celdas, %d redes, %d pines, grilla %dx%d\n",
           nl.num_cells, nl.num_nets, nl.num_pins, grid.nx, grid.ny);

    GpuPlacer *g = gpu_create(&nl, &grid);
    CHECK(g != NULL);
    if (g) {
        float *v = malloc(2 * nl.num_cells * sizeof(float));
        netlist_get_positions(&nl, v);
        test_wirelength(&nl, &grid, g, v);
        test_density(&nl, &grid, g, v);
        free(v);
        gpu_destroy(g);
    }
    grid_free(&grid);
    netlist_free(&nl);
    return finish("test_gpu");
}
