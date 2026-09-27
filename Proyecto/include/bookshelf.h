#ifndef BOOKSHELF_H
#define BOOKSHELF_H

#include "netlist.h"
#include "strmap.h"

#define PATH_LEN 1024

// Lee un benchmark ISPD-2005 a partir del .aux. Regresa 0 si todo salio bien.
int bookshelf_read(const char *aux_path, Netlist *nl);

// Un lector por archivo. Se tienen que llamar en este orden: nodes, nets, pl.
int parse_nodes(const char *path, Netlist *nl, StrMap *names);
int parse_nets(const char *path, Netlist *nl, const StrMap *names);
int parse_pl(const char *path, Netlist *nl, const StrMap *names);

#endif
