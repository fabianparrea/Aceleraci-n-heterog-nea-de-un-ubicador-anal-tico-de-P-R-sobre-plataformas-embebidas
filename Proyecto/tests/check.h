#ifndef CHECK_H
#define CHECK_H

#include <math.h>
#include <stdio.h>

static int failures = 0;

#define CHECK(cond) do { \
    if (!(cond)) { \
        printf("FALLO %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        failures++; \
    } \
} while (0)

#define CHECK_NEAR(a, b, tol) do { \
    double a_ = (a), b_ = (b); \
    if (fabs(a_ - b_) > (tol)) { \
        printf("FALLO %s:%d: %s = %g, esperado %g\n", __FILE__, __LINE__, #a, a_, b_); \
        failures++; \
    } \
} while (0)

static int finish(const char *name)
{
    if (failures == 0)
        printf("%s: ok\n", name);
    else
        printf("%s: %d fallos\n", name, failures);
    return failures != 0;
}

#endif
