#include <cstdio>
#include <mpi.h>
#include <functional>
#include "define.h"

void lab_2_1();

void lab_2_2();

void lab_2_3();

int main() {
    MPI_Init(nullptr, nullptr);

    lab_2_1();
    lab_2_2();
    lab_2_3();

    MPI_Finalize();
}

void calc_time(const std::function<void()> &f, const std::function<void()> &g) {
    double start = MPI_Wtime();
    f();
    double seq_time = MPI_Wtime() - start;
    start = MPI_Wtime();
    g();
    double par_time = MPI_Wtime() - start;

    int comm_sz, my_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    if (my_rank == 0) {
        printf("sequence: %lfs\n", seq_time);
        printf("parallel: %lfs\n", par_time);

        double speedup = seq_time / par_time;
        printf("speedup: %lf\n", speedup);

        double efficiency = speedup / comm_sz;
        printf("efficiency: %lf\n", efficiency);

        printf("\n");
    }
}

// 矩阵-向量乘法
void lab_2_1() {
    int comm_sz, my_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    int n = 600, m = 400;
    double mtx[n * m];
    double vec[m];
    double res[n];
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
}

// 梯形积分法
void lab_2_2() {
    const int n = 200000000;
    double a = 0, b = 1, ans;

    calc_time(
        [&]() { ans = N2::_trap(a, b, n); },
        [&]() { ans = N2::trap(a, b, n); }
    );
}

// 奇偶交换排序
void lab_2_3() {
    int comm_sz, my_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    int n = 500;
    int arr1[n];
    int arr2[n];
    if (my_rank == 0) {
        for (int i = 0; i < n; i++) arr1[i] = i;
        N3::shuffle(arr1, n);
        memcpy(arr2, arr1, sizeof(arr2));
    }
    MPI_Bcast(arr1, n, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(arr2, n, MPI_INT, 0, MPI_COMM_WORLD);

    calc_time(
        [&]() { N3::_odd_even_sort(arr1, n); },
        [&]() { N3::odd_even_sort(arr2, n); }
    );
}
