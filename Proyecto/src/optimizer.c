#include <math.h>
#include "optimizer.h"

// metodo del gradiente optimo de Nesterov: u es la solucion real, v el punto
// "adelantado" donde se evalua el gradiente. La combinacion con coeff es lo que
// le da el impulso (momentum) frente a un descenso por gradiente comun.
void nesterov_step(NesterovState *st, const float *grad)
{
    float a_new = (1.0f + sqrtf(1.0f + 4.0f * st->a * st->a)) / 2.0f;
    float coeff = (st->a - 1.0f) / a_new;

    for (int i = 0; i < st->n2; i++) {
        float u_new = st->v[i] - st->step * grad[i];
        float v_new = u_new + coeff * (u_new - st->u[i]);

        st->v_prev[i] = st->v[i];
        st->grad_prev[i] = grad[i];
        st->u[i] = u_new;
        st->v[i] = v_new;
    }
    st->a = a_new;
}
