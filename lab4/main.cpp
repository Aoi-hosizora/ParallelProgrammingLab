#include <cstdio>
#include <cmath>
#include <mpi.h>
#include <omp.h>

#pragma ide diagnostic ignored "modernize-use-auto"
#pragma ide diagnostic ignored "modernize-use-nullptr"

int comm_sz, my_rank;

double f(double);

void without_mpi();

void with_mpi();

const double a = -100;
const double b = 100;
const int n = 50000000;

int main() {
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    omp_set_num_threads(16);

    // sequential
    double seq_start, seq_end, seq_timespan;
    if (my_rank == 0) {
        seq_start = MPI_Wtime();
        without_mpi();
        seq_end = MPI_Wtime();
        seq_timespan = seq_end - seq_start;
        printf("sequential: %lfs\n", seq_timespan);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    // parallel
    double par_start = MPI_Wtime();
    with_mpi();
    double par_end = MPI_Wtime();
    double par_timespan = par_end - par_start;
    if (my_rank == 0) {
        printf("parallel: %lfs\n", par_timespan);
    }

    if (my_rank == 0) {
        double speedup = seq_timespan / par_timespan;
        printf("speedup: %lf\n", speedup);
        double efficiency = speedup / 32;
        printf("efficiency: %lf\n", efficiency);
    }

    MPI_Finalize();
}

double f(double x) {
    return sin(x) + x * x;
}

void without_mpi() {
    double h = (b - a) / n;
    double integral = (f(a) + f(b)) / 2.0;
    for (int i = 1; i <= n - 1; i++) {
        integral += f(a + i * h);
    }
    integral *= h;

    printf("%lf\n", integral);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedValue"

void with_mpi() {
    // 分割梯形为小梯形的大小
    double h = (b - a) / n;
    // 每个进程计算的小梯形的大小
    int _n = n / comm_sz;
    // 每个进程的梯形的起始位置
    double _a = a + my_rank * _n * h;
    // 每个进程的梯形的结束位置
    double _b = _a + _n * h;

    // 每个线程计算其梯形面积，利用 omp 整合结果到 _int
    double _int = (f(_a) + f(_b)) / 2;
#pragma omp parallel for default(none) shared(_n, _a, h) reduction(+: _int)
    for (int i = 1; i < _n; i++) {
        _int += f(_a + i * h);
    }
    _int *= h;

    // 整合其他进程的结果
    double integral = 0;
    if (my_rank != 0) {
        MPI_Send(&_int, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    } else {
        integral = _int;
        for (int i = 1; i < comm_sz; i++) {
            MPI_Recv(&_int, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            integral += _int;
        }
    }

    // 输出结果
    if (my_rank == 0) {
        printf("%lf\n", integral);
    }
}

#pragma clang diagnostic pop
