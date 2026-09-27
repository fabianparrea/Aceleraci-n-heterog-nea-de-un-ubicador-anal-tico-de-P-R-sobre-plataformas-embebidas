#ifndef STRMAP_H
#define STRMAP_H

// Tabla hash de nombre -> indice. No copia las llaves, el que la usa las mantiene vivas.
typedef struct {
    const char **keys;
    int *vals;
    int cap;
} StrMap;

int strmap_init(StrMap *m, int expected);
int strmap_put(StrMap *m, const char *key, int val);    // 1 si la llave ya estaba
int strmap_get(const StrMap *m, const char *key);       // -1 si no esta
void strmap_free(StrMap *m);

#endif
