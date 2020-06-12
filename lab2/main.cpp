#include <cstdio>
#include <mpi.h>
#include <functional>
#include "define.h"

void lab_2_1();

void lab_2_2();

void lab_2_3();

int comm_sz, my_rank;

int main() {
    MPI_Init(nullptr, nullptr);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    lab_2_1();
    // lab_2_2();
    // lab_2_3();

    MPI_Finalize();
}

void calc_time(const std::function<void()> &f, const std::function<void()> &g) {
    double seq_start, seq_end;
    if (my_rank == 0) {
        seq_start = MPI_Wtime();
        f();
        seq_end = MPI_Wtime();
    }
    double par_start = MPI_Wtime();
    g();
    double par_end = MPI_Wtime();

    if (my_rank == 0) {
        double seq_timespan = seq_end - seq_start;
        double par_timespan = par_end - par_start;

        printf("sequence: %lfs\n", seq_timespan);
        printf("parallel: %lfs\n", par_timespan);

        double speedup = seq_timespan / par_timespan;
        printf("speedup: %lf\n", speedup);

        double efficiency = speedup / comm_sz;
        printf("efficiency: %lf\n", efficiency);

        printf("\n");
    }
}

// 矩阵-向量乘法
void lab_2_1() {
    int n = 10000, m = 10000;
    auto mtx = new double[n * m];
    auto vec = new double[m];
    auto res = new double[n];
    if (my_rank == 0) {
        for (int i = 0; i < n * m; i++) mtx[i] = (i + 1) % 3;
        for (int i = 0; i < m; i++) vec[i] = (i + 1) % 3;
    }

    MPI_Bcast(mtx, n * m, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(vec, m, MPI_INT, 0, MPI_COMM_WORLD);

    calc_time(
        [&]() { N1::_mtx_vec_mul(mtx, vec, res, n, m); },
        [&]() { N1::mtx_vec_mul(mtx, vec, res, n, m); }
    );

    delete[] mtx;
    delete[] vec;
    delete[] res;
}

// 梯形积分法
void lab_2_2() {
    const int n = 5000;
    double a = 0, b = 1, ans;

    calc_time(
        [&]() { ans = N2::_trap(a, b, n); },
        [&]() { ans = N2::trap(a, b, n); }
    );
}

// 奇偶交换排序
void lab_2_3() {
    int n = 50000;
    auto arr1 = new int[n];
    auto arr2 = new int[n];
    if (my_rank == 0) {
        for (int i = 0; i < n; i++) {
            arr1[i] = i;
        }
        N3::shuffle(arr1, n);
        memcpy(arr2, arr1, n * sizeof(int));
    }

    calc_time(
        [&]() { N3::_odd_even_sort(arr1, n); },
        [&]() { N3::odd_even_sort(arr2, n); }
    );

    delete[] arr1;
    delete[] arr2;
}
