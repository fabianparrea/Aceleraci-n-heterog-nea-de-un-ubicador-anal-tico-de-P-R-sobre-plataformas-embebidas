#include <stdio.h>
#include <stdlib.h>
#include "bookshelf.h"
#include "lineio.h"

int parse_nets(const char *path, Netlist *nl, const StrMap *names)
{
    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "error: no se pudo abrir %s\n", path);
        return -1;
    }

    char line[LINE_LEN], name[256], dir;
    int lineno = 0, num_nets = -1, num_pins = -1, pins_left = 0;
    int degree;
    float dx, dy;

    while (next_line(f, line, sizeof line, &lineno)) {
        if (sscanf(line, " NumNets : %d", &num_nets) == 1) {
            nl->nets = calloc(num_nets, sizeof(Net));
            if (!nl->nets) {
                report_error(path, lineno, "sin memoria");
                goto err;
            }
            continue;
        }
        if (sscanf(line, " NumPins : %d", &num_pins) == 1) {
            nl->pins = calloc(num_pins, sizeof(Pin));
            if (!nl->pins) {
                report_error(path, lineno, "sin memoria");
                goto err;
            }
            continue;
        }

        if (sscanf(line, " NetDegree : %d", &degree) == 1) {
            if (pins_left != 0) {
                report_error(path, lineno, "la red anterior tiene pines de menos");
                goto err;
            }
            if (!nl->nets || !nl->pins) {
                report_error(path, lineno, "falta NumNets o NumPins");
                goto err;
            }
            if (nl->num_nets >= num_nets) {
                report_error(path, lineno, "hay mas redes de las declaradas");
                goto err;
            }
            if (degree < 0 || nl->num_pins + degree > num_pins) {
                report_error(path, lineno, "hay mas pines de los declarados");
                goto err;
            }
            Net *net = &nl->nets[nl->num_nets];
            net->id = nl->num_nets;
            net->pin_start = nl->num_pins;
            net->degree = degree;
            nl->num_nets++;
            pins_left = degree;
            continue;
        }

        if (pins_left == 0) {
            report_error(path, lineno, "pin fuera de una red");
            goto err;
        }
        int n = sscanf(line, "%255s %c : %f %f", name, &dir, &dx, &dy);
        if (n != 2 && n != 4) {
            report_error(path, lineno, "linea de pin mal formada");
            goto err;
        }
        if (n == 2)
            dx = dy = 0;

        int cell = strmap_get(names, name);
        if (cell < 0) {
            report_error(path, lineno, "pin apunta a una celda que no existe");
            goto err;
        }
        Pin *p = &nl->pins[nl->num_pins++];
        p->cell = cell;
        p->net = nl->num_nets - 1;
        p->dx = dx;
        p->dy = dy;
        pins_left--;
    }
    fclose(f);

    if (pins_left != 0) {
        fprintf(stderr, "error: %s: la ultima red esta incompleta\n", path);
        return -1;
    }
    if (nl->num_nets != num_nets || nl->num_pins != num_pins) {
        fprintf(stderr, "error: %s: se declararon %d redes y %d pines, hay %d y %d\n",
                path, num_nets, num_pins, nl->num_nets, nl->num_pins);
        return -1;
    }
    return 0;

err:
    fclose(f);
    return -1;
}
