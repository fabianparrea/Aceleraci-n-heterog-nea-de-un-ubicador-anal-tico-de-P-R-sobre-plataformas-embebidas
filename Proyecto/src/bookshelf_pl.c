#include <stdio.h>
#include <string.h>
#include "bookshelf.h"
#include "lineio.h"

int parse_pl(const char *path, Netlist *nl, const StrMap *names)
{
    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "error: no se pudo abrir %s\n", path);
        return -1;
    }

    char line[LINE_LEN], name[256];
    int lineno = 0, count = 0;
    float x, y;

    while (next_line(f, line, sizeof line, &lineno)) {
        if (sscanf(line, "%255s %f %f", name, &x, &y) != 3) {
            report_error(path, lineno, "linea de posicion mal formada");
            fclose(f);
            return -1;
        }
        int id = strmap_get(names, name);
        if (id < 0) {
            report_error(path, lineno, "celda desconocida");
            fclose(f);
            return -1;
        }
        Cell *c = &nl->cells[id];
        c->x = x;
        c->y = y;
        if (strstr(line, "/FIXED"))
            c->fixed = 1;
        count++;
    }
    fclose(f);

    if (count != nl->num_cells) {
        fprintf(stderr, "error: %s: hay %d posiciones y %d celdas\n",
                path, count, nl->num_cells);
        return -1;
    }
    return 0;
}
