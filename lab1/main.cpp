#include <cstdio>
#include <omp.h>
#include <functional>
#include "define.h"

void lab_1_1();

void lab_1_2();

void lab_1_3();

int main() {
    omp_set_num_threads(8);
    lab_1_1();
    printf("\n");
    lab_1_2();
    printf("\n");
    lab_1_3();
}

void calc_time(const std::function<void()> &f, const std::function<void()> &g) {
    double start = omp_get_wtime();
    f();
    double seq_time = omp_get_wtime() - start;
    printf("sequence: %lfs\n", seq_time);
    start = omp_get_wtime();
    g();
    double par_time = omp_get_wtime() - start;
    printf("parallel: %lfs\n", par_time);

    double speedup = seq_time / par_time;
    printf("speedup: %lf\n", speedup);

    int thread_nums;
#pragma omp parallel default(none) shared(thread_nums)
    {
#pragma omp single
        thread_nums = omp_get_num_threads();
    }

    double efficiency = speedup / thread_nums;
    printf("efficiency: %lf\n", efficiency);
}

// 矩阵-向量乘法
void lab_1_1() {
    int n = 600, m = 400;
    double mtx[n * m];
    double vec[m];
    double res[n];
    for (int i = 0; i < n * m; i++) mtx[i] = (i + 1) % 3;
    for (int i = 0; i < m; i++) vec[i] = (i + 1) % 3;

    calc_time(
        [&]() { N1::_mtx_vec_mul(mtx, vec, res, n, m); },
        [&]() { N1::mtx_vec_mul(mtx, vec, res, n, m); }
    );
}

// 梯形积分法
void lab_1_2() {
    const int n = 200000000;
    double a = 0, b = 1, ans;

    calc_time([&]() { ans = N2::_trap(a, b, n); }, [&]() { ans = N2::trap(a, b, n); });
}

// 奇偶交换排序
void lab_1_3() {
    int n = 20000;
    int arr1[n];
    int arr2[n];
    for (int i = 0; i < n; i++) arr1[i] = i;
    for (int i = 0; i < n; i++) arr2[i] = i;
    N3::shuffle(arr1, n);
    N3::shuffle(arr2, n);

    calc_time(
        [&]() { N3::_odd_even_sort(arr1, n); },
        [&]() { N3::odd_even_sort(arr2, n); }
    );
}
