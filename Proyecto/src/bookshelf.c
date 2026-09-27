#include <stdio.h>
#include <string.h>
#include "bookshelf.h"
#include "lineio.h"

// Saca del .aux los nombres de los .nodes, .nets, .pl y .scl (este ultimo es
// opcional: sin el no se puede legalizar, pero si se puede ubicar).
static int read_aux(const char *aux_path, char *nodes, char *nets, char *pl, char *scl, size_t size)
{
    FILE *f = fopen(aux_path, "r");
    if (!f) {
        fprintf(stderr, "error: no se pudo abrir %s\n", aux_path);
        return -1;
    }

    // los archivos estan en la misma carpeta del .aux
    char dir[PATH_LEN] = "";
    const char *slash = strrchr(aux_path, '/');
    if (slash)
        snprintf(dir, sizeof dir, "%.*s", (int)(slash - aux_path + 1), aux_path);

    char line[LINE_LEN];
    int lineno = 0;
    nodes[0] = nets[0] = pl[0] = scl[0] = '\0';

    while (next_line(f, line, sizeof line, &lineno)) {
        char *colon = strchr(line, ':');
        if (!colon)
            continue;
        for (char *tok = strtok(colon + 1, " \t\r\n"); tok; tok = strtok(NULL, " \t\r\n")) {
            char *dot = strrchr(tok, '.');
            if (!dot)
                continue;
            if (strcmp(dot, ".nodes") == 0)
                snprintf(nodes, size, "%s%s", dir, tok);
            else if (strcmp(dot, ".nets") == 0)
                snprintf(nets, size, "%s%s", dir, tok);
            else if (strcmp(dot, ".pl") == 0)
                snprintf(pl, size, "%s%s", dir, tok);
            else if (strcmp(dot, ".scl") == 0)
                snprintf(scl, size, "%s%s", dir, tok);
        }
    }
    fclose(f);

    if (!nodes[0] || !nets[0] || !pl[0]) {
        fprintf(stderr, "error: %s no lista los .nodes, .nets y .pl\n", aux_path);
        return -1;
    }
    return 0;
}

int bookshelf_read(const char *aux_path, Netlist *nl)
{
    char nodes[PATH_LEN], nets[PATH_LEN], pl[PATH_LEN], scl[PATH_LEN];
    StrMap names = {0};

    memset(nl, 0, sizeof *nl);
    if (read_aux(aux_path, nodes, nets, pl, scl, PATH_LEN) < 0)
        return -1;

    if (parse_nodes(nodes, nl, &names) < 0)
        goto fail;
    if (parse_nets(nets, nl, &names) < 0)
        goto fail;
    if (parse_pl(pl, nl, &names) < 0)
        goto fail;
    if (scl[0] && parse_scl(scl, nl) < 0)
        goto fail;

    strmap_free(&names);
    return 0;

fail:
    strmap_free(&names);
    netlist_free(nl);
    return -1;
}
