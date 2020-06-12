#include <mpi.h>
#include <cstdio>
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

    printf("%lf\n", integral);
    return integral;
}

double N2::trap(double a, double b, int n) {
    int comm_sz, my_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    double ans = 0;

    // 根据当前的 rank 拆分每个进程的计算
    double h = (b - a) / n;
    int _n = n / comm_sz;
    double _a = a + my_rank * _n * h;
    double _b = _a + _n * h;

    // 每个进程的具体计算过程
    double integral = (f(_a) + f(_b)) / 2;
    for (int i = 1; i <= _n - 1; i++) {
        integral += f(_a + i * h);
    }
    integral *= h;

    if (my_rank != 0) {
        // 当前如果是非 0 进程，将计算结果传给 0 进程处理
        MPI_Send(&integral, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    } else {
        ans = integral;
        double tmp;
        // 对每个进程的计算结果叠加
        for (int i = 1; i < comm_sz; i++) {
            MPI_Recv(&tmp, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            ans += tmp;
        }
    }

    if (my_rank == 0) {
        printf("%lf\n", ans);
    }
    return ans;
}
