#ifndef MEDIAN_H
#define MEDIAN_H

// El k-esimo menor (k desde 0) de |v[0]|, ..., |v[n-1]|: el mismo valor que
// quedaria en la posicion k si se ordenaran las magnitudes. No modifica v.
float select_abs_kth(const float *v, int n, int k);

// mediana de las magnitudes, la que usa la calibracion de lambda
float median_abs(const float *v, int n);

#endif
