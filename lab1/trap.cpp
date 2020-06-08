#include <omp.h>
#include "define.h"

double f(double a) {
    return a * a * a;
}

double N2::_trap(double a, double b, int n) {
    int i;

    double h = (b - a) / n;
    double integral = (f(a) + f(b)) / 2.0;
    for (i = 1; i <= n - 1; i++) {
        integral += f(a + i * h);
    }
    integral *= h;

    // printf("%lf\n", integral);
    return integral;
}

double N2::trap(double a, double b, int n) {
    double ans = 0;
    auto _func = [](
        double a, /* in */
        double b, /* in */
        int n /* in */
    ) {
        int i;

        double h = (b - a) / n;
        int _n = n / omp_get_num_threads();
        double _a = a + omp_get_thread_num() * _n * h;
        double _b = _a + _n * h;
        double integral = (f(_a) + f(_b)) / 2;
        for (i = 1; i <= _n - 1; i++) {
            integral += f(_a + i * h);
        }
        integral *= h;
        return integral;
    };

#pragma omp parallel default(none) reduction(+: ans) shared(n, a, b, _func)
    ans += _func(a, b, n);

    // printf("%lf\n", ans);
    return ans;
}
