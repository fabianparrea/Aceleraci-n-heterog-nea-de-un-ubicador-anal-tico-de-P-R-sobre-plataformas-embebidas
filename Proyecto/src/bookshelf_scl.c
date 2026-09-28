#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bookshelf.h"
#include "lineio.h"

int parse_scl(const char *path, Netlist *nl)
{
    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "error: no se pudo abrir %s\n", path);
        return -1;
    }

    char line[LINE_LEN];
    int lineno = 0, num_rows = -1, in_row = 0;
    float y = 0, height = 0, site_w = 0, x0 = 0;
    int num_sites = 0;

    while (next_line(f, line, sizeof line, &lineno)) {
        if (sscanf(line, " NumRows : %d", &num_rows) == 1) {
            nl->rows = calloc(num_rows, sizeof(Row));
            if (!nl->rows) {
                report_error(path, lineno, "sin memoria");
                goto err;
            }
            continue;
        }
        if (strncmp(line, "CoreRow", 7) == 0) {
            in_row = 1;
            continue;
        }
        if (strncmp(line, "End", 3) == 0) {
            if (!in_row || !nl->rows) {
                report_error(path, lineno, "End sin CoreRow");
                goto err;
            }
            if (nl->num_rows >= num_rows) {
                report_error(path, lineno, "hay mas filas de las declaradas");
                goto err;
            }
            Row *r = &nl->rows[nl->num_rows++];
            r->x0 = x0;
            r->y0 = y;
            r->height = height;
            r->site_w = site_w;
            r->num_sites = num_sites;
            in_row = 0;
            continue;
        }
        if (sscanf(line, " Coordinate : %f", &y) == 1) continue;
        if (sscanf(line, " Height : %f", &height) == 1) continue;
        if (sscanf(line, " Sitewidth : %f", &site_w) == 1) continue;
        if (sscanf(line, " SubrowOrigin : %f NumSites : %d", &x0, &num_sites) == 2) continue;
        // Sitespacing/Siteorient/Sitesymmetry: no hacen falta para legalizar
    }
    fclose(f);

    if (nl->num_rows != num_rows) {
        fprintf(stderr, "error: %s: NumRows es %d pero hay %d filas\n",
                path, num_rows, nl->num_rows);
        return -1;
    }
    return 0;

err:
    fclose(f);
    return -1;
}
