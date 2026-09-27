#include <ctype.h>
#include <string.h>
#include "lineio.h"

int next_line(FILE *f, char *buf, int size, int *lineno)
{
    while (fgets(buf, size, f)) {
        (*lineno)++;

        // si la linea no cabe se bota lo que sobra
        if (!strchr(buf, '\n') && !feof(f)) {
            int c;
            while ((c = fgetc(f)) != '\n' && c != EOF)
                ;
        }

        char *p = buf;
        while (isspace((unsigned char)*p))
            p++;
        // "UCLA " con espacio: asi no se confunde con una celda que se llame UCLAalgo
        if (*p == '\0' || *p == '#' || strncmp(p, "UCLA ", 5) == 0)
            continue;
        return 1;
    }
    return 0;
}

void report_error(const char *path, int lineno, const char *msg)
{
    fprintf(stderr, "error: %s:%d: %s\n", path, lineno, msg);
}
