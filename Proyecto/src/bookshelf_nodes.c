#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bookshelf.h"
#include "lineio.h"

int parse_nodes(const char *path, Netlist *nl, StrMap *names)
{
    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "error: no se pudo abrir %s\n", path);
        return -1;
    }

    char line[LINE_LEN], name[256], kind[32];
    int lineno = 0, num_nodes = -1, num_terminals = -1, terminals = 0;
    float w, h;

    while (next_line(f, line, sizeof line, &lineno)) {
        if (sscanf(line, " NumNodes : %d", &num_nodes) == 1) {
            nl->cells = calloc(num_nodes, sizeof(Cell));
            if (!nl->cells || strmap_init(names, num_nodes) < 0) {
                report_error(path, lineno, "sin memoria");
                goto err;
            }
            continue;
        }
        if (sscanf(line, " NumTerminals : %d", &num_terminals) == 1)
            continue;

        if (!nl->cells) {
            report_error(path, lineno, "falta NumNodes antes de los nodos");
            goto err;
        }
        kind[0] = '\0';
        int n = sscanf(line, "%255s %f %f %31s", name, &w, &h, kind);
        if (n < 3) {
            report_error(path, lineno, "linea de nodo mal formada");
            goto err;
        }
        if (nl->num_cells >= num_nodes) {
            report_error(path, lineno, "hay mas nodos de los declarados");
            goto err;
        }

        Cell *c = &nl->cells[nl->num_cells];
        c->id = nl->num_cells;
        c->name = strdup(name);
        c->width = w;
        c->height = h;
        c->area = w * h;
        c->fixed = strncmp(kind, "terminal", 8) == 0;
        nl->num_cells++;
        if (!c->name) {
            report_error(path, lineno, "sin memoria");
            goto err;
        }
        if (strmap_put(names, c->name, c->id) != 0) {
            report_error(path, lineno, "nombre de nodo repetido");
            goto err;
        }
        terminals += c->fixed;
    }
    fclose(f);

    if (nl->num_cells != num_nodes) {
        fprintf(stderr, "error: %s: NumNodes es %d pero hay %d nodos\n",
                path, num_nodes, nl->num_cells);
        return -1;
    }
    if (num_terminals >= 0 && terminals != num_terminals) {
        fprintf(stderr, "error: %s: NumTerminals es %d pero hay %d\n",
                path, num_terminals, terminals);
        return -1;
    }
    return 0;

err:
    fclose(f);
    return -1;
}
