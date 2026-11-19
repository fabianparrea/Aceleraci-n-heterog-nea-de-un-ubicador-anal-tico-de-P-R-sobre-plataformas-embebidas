#include <math.h>
#include "check.h"
#include "optimizer.h"

// minimizar f(x) = x^2 (grad = 2x) tiene que converger a x = 0
static void test_nesterov_converges(void)
{
    float u[1] = {10.0f}, v[1] = {10.0f}, v_prev[1] = {10.0f}, grad_prev[1] = {0.0f};
    NesterovState st = {1, u, v, v_prev, grad_prev, 1.0f, 0.1f, 0.0f};

    for (int i = 0; i < 200; i++) {
        float grad[1] = { 2.0f * st.v[0] };
        nesterov_step(&st, grad);
    }
    CHECK_NEAR(st.u[0], 0.0, 1e-2);
}

// la recurrencia del coeficiente de Nesterov es a_(k+1) = (1 + sqrt(1 + 4*a_k^2)) / 2
static void test_nesterov_a_sequence(void)
{
    float u[1] = {0}, v[1] = {0}, v_prev[1] = {0}, grad_prev[1] = {0};
    NesterovState st = {1, u, v, v_prev, grad_prev, 1.0f, 0.0f, 0.0f};
    float grad[1] = {0.0f};

    nesterov_step(&st, grad);
    CHECK_NEAR(st.a, (1.0 + sqrt(5.0)) / 2.0, 1e-5);

    float a1 = st.a;
    nesterov_step(&st, grad);
    CHECK_NEAR(st.a, (1.0 + sqrt(1.0 + 4.0 * a1 * a1)) / 2.0, 1e-5);
}

// con limites, un paso que manda la coordenada afuera la deja en el borde
static void test_nesterov_bounds(void)
{
    float u[2] = {0.5f, 0.5f}, v[2] = {0.5f, 0.5f}, v_prev[2], grad_prev[2];
    float lo[2] = {0.0f, 0.0f}, hi[2] = {1.0f, 1.0f};
    NesterovState st = {2, u, v, v_prev, grad_prev, 1.0f, 1.0f, 0.0f, lo, hi};
    float grad[2] = {-10.0f, 10.0f};

    nesterov_step(&st, grad);
    CHECK_NEAR(st.u[0], 1.0, 1e-6);
    CHECK_NEAR(st.v[0], 1.0, 1e-6);
    CHECK_NEAR(st.u[1], 0.0, 1e-6);
    CHECK_NEAR(st.v[1], 0.0, 1e-6);
}

int main(void)
{
    test_nesterov_converges();
    test_nesterov_a_sequence();
    test_nesterov_bounds();
    return finish("test_optimizer");
}
