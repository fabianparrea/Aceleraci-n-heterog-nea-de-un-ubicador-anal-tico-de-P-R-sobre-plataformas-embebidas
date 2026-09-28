#include <stdlib.h>
#include <string.h>
#include "strmap.h"

static unsigned hash_str(const char *s)
{
    unsigned h = 2166136261u;
    while (*s) {
        h ^= (unsigned char)*s++;
        h *= 16777619u;
    }
    return h;
}

int strmap_init(StrMap *m, int expected)
{
    int cap = 16;
    while (cap < 2 * expected)
        cap *= 2;

    m->keys = calloc(cap, sizeof(char *));
    m->vals = malloc(cap * sizeof(int));
    m->cap = cap;
    if (!m->keys || !m->vals) {
        strmap_free(m);
        return -1;
    }
    return 0;
}

int strmap_put(StrMap *m, const char *key, int val)
{
    unsigned i = hash_str(key) & (m->cap - 1);
    while (m->keys[i]) {
        if (strcmp(m->keys[i], key) == 0)
            return 1;
        i = (i + 1) & (m->cap - 1);
    }
    m->keys[i] = key;
    m->vals[i] = val;
    return 0;
}

int strmap_get(const StrMap *m, const char *key)
{
    unsigned i = hash_str(key) & (m->cap - 1);
    while (m->keys[i]) {
        if (strcmp(m->keys[i], key) == 0)
            return m->vals[i];
        i = (i + 1) & (m->cap - 1);
    }
    return -1;
}

void strmap_free(StrMap *m)
{
    free(m->keys);
    free(m->vals);
    m->keys = NULL;
    m->vals = NULL;
    m->cap = 0;
}
