#ifndef LINEIO_H
#define LINEIO_H

#include <stdio.h>

#define LINE_LEN 1024

// Lee la siguiente linea con datos. Salta vacias, comentarios y el header UCLA.
// Regresa 0 cuando se acaba el archivo.
int next_line(FILE *f, char *buf, int size, int *lineno);

void report_error(const char *path, int lineno, const char *msg);

#endif
