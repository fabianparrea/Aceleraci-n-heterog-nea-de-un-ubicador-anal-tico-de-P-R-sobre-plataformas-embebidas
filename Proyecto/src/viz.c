#include <float.h>
#include <stdio.h>
#include <stdlib.h>
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

    FILE *f = fopen(path, "wb");
    if (f) {
        fprintf(f, "P6\n%d %d\n255\n", w, h);
        fwrite(img, 1, (size_t)w * h * 3, f);
        fclose(f);
    }
    free(img);
}
