#ifndef OPTIMIZER_H
#define OPTIMIZER_H

typedef struct {
    int n2;             // 2N
    float *u;           // solucion actual
    float *v;           // punto de referencia
    float *v_prev;
    float *grad_prev;
    float a;            // coeficiente de Nesterov
    float step;
    float lambda;       // peso de la densidad
    const float *lo;    // limites de cada coordenada de u y v (NULL = sin
    const float *hi;    // limites)
} NesterovState;

// grad ya viene combinado: grad_W + lambda * grad_D
void nesterov_step(NesterovState *st, const float *grad);

#endif
