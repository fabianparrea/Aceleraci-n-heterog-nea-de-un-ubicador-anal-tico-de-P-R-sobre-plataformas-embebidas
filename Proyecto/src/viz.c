#include <errno.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "viz.h"

static void fill_rect(unsigned char *img, int w, int h, int x0, int x1, int y0, int y1,
                      unsigned char r, unsigned char g, unsigned char b)
{
    if (x1 <= x0) x1 = x0 + 1;
    if (y1 <= y0) y1 = y0 + 1;
    for (int y = y0; y <= y1 && y < h; y++) {
        if (y < 0) continue;
        int row = h - 1 - y;   // la imagen crece hacia abajo, el chip hacia arriba
        for (int x = x0; x <= x1 && x < w; x++) {
            if (x < 0) continue;
            size_t idx = ((size_t)row * w + x) * 3;
            img[idx] = r;
            img[idx + 1] = g;
            img[idx + 2] = b;
        }
    }
}

static int write_ppm(const char *path, const unsigned char *img, int w, int h)
{
    FILE *f = fopen(path, "wb");
    if (!f)
        return -1;
    fprintf(f, "P6\n%d %d\n255\n", w, h);
    size_t bytes = (size_t)w * h * 3;
    int ok = fwrite(img, 1, bytes, f) == bytes;
    fclose(f);
    return ok ? 0 : -1;
}

void write_layout_ppm(const char *path, const Netlist *nl, const float *v, int max_dim)
{
    int n = nl->num_cells;
    float xmin = FLT_MAX, xmax = -FLT_MAX, ymin = FLT_MAX, ymax = -FLT_MAX;
    for (int i = 0; i < n; i++) {
        const Cell *c = &nl->cells[i];
        float x0 = v[i] - c->width / 2, x1 = v[i] + c->width / 2;
        float y0 = v[n + i] - c->height / 2, y1 = v[n + i] + c->height / 2;
        if (x0 < xmin) xmin = x0;
        if (x1 > xmax) xmax = x1;
        if (y0 < ymin) ymin = y0;
        if (y1 > ymax) ymax = y1;
    }
    float die_w = xmax - xmin > 0 ? xmax - xmin : 1;
    float die_h = ymax - ymin > 0 ? ymax - ymin : 1;
    int w = die_w >= die_h ? max_dim : (int)(max_dim * die_w / die_h);
    int h = die_w >= die_h ? (int)(max_dim * die_h / die_w) : max_dim;
    if (w < 1) w = 1;
    if (h < 1) h = 1;

    unsigned char *img = calloc((size_t)w * h * 3, 1);
    if (!img)
        return;

    float sx = (w - 1) / die_w, sy = (h - 1) / die_h;
    for (int i = 0; i < n; i++) {
        const Cell *c = &nl->cells[i];
        int x0 = (int)((v[i] - c->width / 2 - xmin) * sx);
        int x1 = (int)((v[i] + c->width / 2 - xmin) * sx);
        int y0 = (int)((v[n + i] - c->height / 2 - ymin) * sy);
        int y1 = (int)((v[n + i] + c->height / 2 - ymin) * sy);
        if (c->fixed)
            fill_rect(img, w, h, x0, x1, y0, y1, 220, 60, 60);
        else
            fill_rect(img, w, h, x0, x1, y0, y1, 40, 160, 220);
    }
    write_ppm(path, img, w, h);
    free(img);
}

struct VizFrames {
    const Netlist *nl;
    char dir[512];
    FILE *csv;
    char name[64];
    int every;
    int count;
    int w, h;
    float x0, y0, scale;    // de coordenadas del chip a pixeles
    float *cover;           // cuanto de cada pixel tapan las movibles (1 = lleno)
    float *fixed;           // lo mismo, pero de las fijas
    unsigned char *img;
};

// mezcla lineal entre dos colores, t entre 0 y 1
static void blend(unsigned char *dst, const unsigned char *a, const unsigned char *b, float t)
{
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    for (int k = 0; k < 3; k++)
        dst[k] = (unsigned char)(a[k] + (b[k] - a[k]) * t + 0.5f);
}

VizFrames *viz_frames_open(const char *dir, const char *name, const Netlist *nl,
                           int every, int max_dim)
{
    if (mkdir(dir, 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "no se pudo crear la carpeta de cuadros %s\n", dir);
        return NULL;
    }
    VizFrames *f = calloc(1, sizeof *f);
    if (!f)
        return NULL;
    f->nl = nl;
    f->every = every > 0 ? every : 1;
    snprintf(f->dir, sizeof f->dir, "%s", dir);
    snprintf(f->name, sizeof f->name, "%s", name);

    // el recuadro es el area de las filas, que es donde terminan las celdas;
    // sin .scl se usa lo que ocupan las celdas en el .pl
    float x0 = FLT_MAX, y0 = FLT_MAX, x1 = -FLT_MAX, y1 = -FLT_MAX;
    for (int k = 0; k < nl->num_rows; k++) {
        const Row *r = &nl->rows[k];
        x0 = fminf(x0, r->x0);
        y0 = fminf(y0, r->y0);
        x1 = fmaxf(x1, r->x0 + r->num_sites * r->site_w);
        y1 = fmaxf(y1, r->y0 + r->height);
    }
    if (nl->num_rows == 0) {
        for (int i = 0; i < nl->num_cells; i++) {
            const Cell *c = &nl->cells[i];
            x0 = fminf(x0, c->x);
            y0 = fminf(y0, c->y);
            x1 = fmaxf(x1, c->x + c->width);
            y1 = fmaxf(y1, c->y + c->height);
        }
    }
    float die_w = x1 - x0 > 0 ? x1 - x0 : 1;
    float die_h = y1 - y0 > 0 ? y1 - y0 : 1;
    f->w = die_w >= die_h ? max_dim : (int)(max_dim * die_w / die_h);
    f->h = die_w >= die_h ? (int)(max_dim * die_h / die_w) : max_dim;
    if (f->w < 1) f->w = 1;
    if (f->h < 1) f->h = 1;
    f->x0 = x0;
    f->y0 = y0;
    f->scale = f->w / die_w;

    size_t px = (size_t)f->w * f->h;
    f->cover = malloc(px * sizeof(float));
    f->fixed = malloc(px * sizeof(float));
    f->img = malloc(px * 3);
    char path[600];
    snprintf(path, sizeof path, "%s/cuadros.csv", dir);
    f->csv = fopen(path, "w");
    if (!f->cover || !f->fixed || !f->img || !f->csv) {
        fprintf(stderr, "no se pudo preparar la animacion en %s\n", dir);
        viz_frames_close(f);
        return NULL;
    }
    fprintf(f->csv, "cuadro,circuito,etapa,iteracion,hpwl,overflow\n");
    return f;
}

int viz_frames_due(const VizFrames *f, int iter)
{
    return f && iter % f->every == 0;
}

// suma en buf la parte de cada pixel que tapa el rectangulo [x0,x1]x[y0,y1]
// (en pixeles, con decimales); una celda chica tapa solo una fraccion
static void add_cover(const VizFrames *f, float *buf, float x0, float x1, float y0, float y1)
{
    int ix0 = (int)floorf(x0), ix1 = (int)floorf(x1);
    int iy0 = (int)floorf(y0), iy1 = (int)floorf(y1);
    if (ix0 < 0) ix0 = 0;
    if (iy0 < 0) iy0 = 0;
    if (ix1 >= f->w) ix1 = f->w - 1;
    if (iy1 >= f->h) iy1 = f->h - 1;
    for (int y = iy0; y <= iy1; y++) {
        float oy = fminf(y1, y + 1.0f) - fmaxf(y0, (float)y);
        if (oy <= 0.0f)
            continue;
        float *row = buf + (size_t)(f->h - 1 - y) * f->w;
        for (int x = ix0; x <= ix1; x++) {
            float ox = fminf(x1, x + 1.0f) - fmaxf(x0, (float)x);
            if (ox > 0.0f)
                row[x] += ox * oy;
        }
    }
}

void viz_frames_add(VizFrames *f, const float *v, const char *stage, int iter,
                    double hpwl, float overflow)
{
    if (!f)
        return;
    static const unsigned char bg[3] = {18, 20, 26};
    static const unsigned char blue[3] = {40, 160, 220};
    static const unsigned char red[3] = {120, 44, 50};
    static const unsigned char hot[3] = {255, 214, 80};

    const Netlist *nl = f->nl;
    int n = nl->num_cells;
    size_t px = (size_t)f->w * f->h;
    memset(f->cover, 0, px * sizeof(float));
    memset(f->fixed, 0, px * sizeof(float));
    for (int i = 0; i < n; i++) {
        const Cell *c = &nl->cells[i];
        float x0 = (v[i] - c->width / 2 - f->x0) * f->scale;
        float x1 = (v[i] + c->width / 2 - f->x0) * f->scale;
        float y0 = (v[n + i] - c->height / 2 - f->y0) * f->scale;
        float y1 = (v[n + i] + c->height / 2 - f->y0) * f->scale;
        add_cover(f, c->fixed ? f->fixed : f->cover, x0, x1, y0, y1);
    }

    // un pixel que es mas que nada de una fija va en rojo; si no, del fondo
    // al azul hasta llenarse. Lo que no cabe entre fijas y movibles lo va
    // llevando al amarillo. La raiz es para que una zona con pocas celdas
    // no quede casi negra.
    for (size_t p = 0; p < px; p++) {
        float c = f->cover[p];
        float fx = fminf(f->fixed[p], 1.0f);
        float extra = c + fx - 1.0f;
        unsigned char *dst = f->img + p * 3;
        if (fx >= 0.5f)
            blend(dst, red, hot, extra > 0.0f ? sqrtf(extra) : 0.0f);
        else if (extra <= 0.0f)
            blend(dst, bg, blue, sqrtf(c));
        else
            blend(dst, blue, hot, extra / 1.5f);
    }

    char path[600];
    snprintf(path, sizeof path, "%s/cuadro_%04d.ppm", f->dir, f->count);
    if (write_ppm(path, f->img, f->w, f->h) != 0) {
        fprintf(stderr, "no se pudo escribir %s\n", path);
        return;
    }
    fprintf(f->csv, "%d,%s,%s,", f->count, f->name, stage);
    if (iter >= 0)
        fprintf(f->csv, "%d", iter);
    fprintf(f->csv, ",%.6e,", hpwl);
    if (overflow >= 0.0f)
        fprintf(f->csv, "%.4f", overflow);
    fprintf(f->csv, "\n");
    f->count++;
}

void viz_frames_close(VizFrames *f)
{
    if (!f)
        return;
    if (f->csv)
        fclose(f->csv);
    free(f->cover);
    free(f->fixed);
    free(f->img);
    free(f);
}
