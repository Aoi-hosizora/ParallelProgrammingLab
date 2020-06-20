#include <cstdio>
#include <cmath>
#include <algorithm>
// #include <random>
// #include <ctime>
#include <mpi.h>
#include <omp.h>
#include <cstring>

#pragma ide diagnostic ignored "modernize-use-auto"
#pragma ide diagnostic ignored "modernize-use-nullptr"

int comm_sz, my_rank;

void without_mpi();

void with_mpi();

int n = 100000, m = 1000;
double *mtx;
double *vec;

int main() {
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    omp_set_num_threads(8);

    // data
    mtx = new double[n * m]; // n * m
    vec = new double[m]; // m * 1
    if (my_rank == 0) {
        for (int i = 0; i < n * m; i++) mtx[i] = 20 + i % 80;
        for (int i = 0; i < m; i++) vec[i] = 20 + i % 80;
        // unsigned seed = time(0);
        // std::shuffle(mtx, mtx + n * m, std::default_random_engine(seed));
        // std::shuffle(vec, vec + m, std::default_random_engine(seed));
    }

    // sequential
    double seq_start, seq_end, seq_timespan;
    if (my_rank == 0) {
        seq_start = MPI_Wtime();
        without_mpi();
        seq_end = MPI_Wtime();
        seq_timespan = seq_end - seq_start;
        printf("sequential: %lfs\n", seq_timespan);
    }

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
        double efficiency = speedup / 8; // comm_sz
        printf("efficiency: %lf\n", efficiency);
    }

    delete[] mtx;
    delete[] vec;

    MPI_Finalize();
}

void without_mpi() {
    double *res = new double[n]; // n * 1

    for (int i = 0; i < n; i++) {
        res[i] = 0;
        for (int j = 0; j < m; j++) {
            res[i] += mtx[i * m + j] * vec[j];
        }
    }

    for (int i = 0; i < 5; i++) {
        printf("%lf ", res[i]);
    }
    printf("\n");

    delete[] res;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedValue"

void with_mpi() {
    // 需要进程数能够整除矩阵行数
    int _length = floor((double) n / comm_sz);
    // 每个进程的矩阵
    double *_mtx = new double[_length * m];

    // 为每个进程发布其子矩阵
    MPI_Scatter(mtx, _length, MPI_DOUBLE, _mtx, _length, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    // 广播向量
    MPI_Bcast(vec, m, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // 记录每个进程的计算结果
    double *_res = new double[_length]; // n * 1
    memset(_res, 0, _length * sizeof(double));

    // 多线程，对每一行的计算并发进行
#pragma omp parallel for default(none) shared(_length, _res, _mtx, vec, m)
    for (int r = 0; r < _length; r++) {
        int s = r * m;
        for (int c = 0; c < m; c++) {
            _res[r] += _mtx[s + c] * vec[c];
        }
    }

    // 最终结果，汇总到 0 号进程
    double *res = new double[n];
    MPI_Gather(_res, _length, MPI_DOUBLE, res, _length, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // 输出结果
    if (my_rank == 0) {
        for (int i = 0; i < 5; i++) {
            printf("%lf ", res[i]);
        }
        printf("\n");
    }

    delete[] _mtx;
    delete[] _res;
    delete[] res;
}

#pragma clang diagnostic pop
