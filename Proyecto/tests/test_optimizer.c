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

int main(void)
{
    test_nesterov_converges();
    test_nesterov_a_sequence();
    return finish("test_optimizer");
}
