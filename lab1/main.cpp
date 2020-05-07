#include <cstdio>
#include <omp.h>
#include <algorithm>
#include <random>
#include <ctime>

typedef long long LL;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedGlobalDeclarationInspection"
#pragma ide diagnostic ignored "bugprone-branch-clone"
// #pragma ide diagnostic ignored "UnusedValue"

void lab_1_1();

void lab_1_2();

void lab_1_3();

void mtx_vec_mul(const float *, const float *, float *, int, int);

double f(double);

double trap(double, double, LL);

void odd_even_sort(int[], int);

int main() {
    lab_1_1();
    printf("\n");
    lab_1_2();
    printf("\n");
    lab_1_3();
}

// 矩阵-向量乘法
void lab_1_1() {
    time_t time;
    auto print = [](const float res[], int n) {
        // double ans = 0;
        // for (int i = 0; i < n; i++) {
        //     printf("%lf ", res[i]);
        // }
        // printf("\n");
    };

    int n = 300, m = 200;
    float mtx[n * m];
    float vec[m];
    float res[n];
    for (int i = 0; i < n * m; i++) mtx[i] = (float) ((i + 1) % 3);
    for (int i = 0; i < m; i++) vec[i] = (float) ((i + 1) % 3);

    // 0.
    time = std::clock();
    mtx_vec_mul(mtx, vec, res, n, m);
    print(res, n);
    printf("%lfs\n", (float) (clock() - time) / CLOCKS_PER_SEC);

    // 1.
    time = std::clock();
    int i, j;
#pragma omp parallel for default(none) shared(mtx, vec, res, n, m)
    for (i = 0; i < n; i++) {
        res[i] = 0;
#pragma omp parallel for default(none) shared(i, mtx, res, vec, m)
        for (j = 0; j < m; j++) {
            res[i] += mtx[i * m + j] * vec[j];
        }
    }
    print(res, n);
    printf("%lfs\n", (float) (clock() - time) / CLOCKS_PER_SEC);

    // 2.
    time = std::clock();
#pragma omp parallel for default(none) shared(mtx, vec, res, n, m)
    for (i = 0; i < n; i++) {
        res[i] = 0;
#pragma omp parallel for default(none) reduction(+: res[i]) shared(i, mtx, vec, m)
        for (j = 0; j < m; j++) {
            res[i] += mtx[i * m + j] * vec[j];
        }
    }
    print(res, n);
    printf("%lfs\n", (float) (clock() - time) / CLOCKS_PER_SEC);

    // 3.
    time = std::clock();
#pragma omp parallel for schedule(static) default(none) shared(mtx, vec, res, n, m) private(i, j)
    for (i = 0; i < n; i++) {
        res[i] = 0;
        for (j = 0; j < m; j++) {
            res[i] += mtx[i * m + j] * vec[j];
        }
    }
    print(res, n);
    printf("%lfs\n", (float) (clock() - time) / CLOCKS_PER_SEC);
}

void mtx_vec_mul(
    const float *mtx, /* in */
    const float *vec, /* in */
    float *res,  /* out */
    int n, /* in */
    int m /* in */
) {
    int i, j;
    for (i = 0; i < n; i++) {
        res[i] = 0;
        for (j = 0; j < m; j++) {
            res[i] += mtx[i * m + j] * vec[j];
        }
    }
}

// 梯形积分法
void lab_1_2() {
    time_t time;
    auto print = [](double ans) {
        // printf("%lf ", ans);
    };
    LL n = 100000000;
    double a = 0, b = 1, ans;

    // 0.
    time = std::clock();
    ans = trap(a, b, n);
    print(ans);
    printf("%lfs\n", (float) (clock() - time) / CLOCKS_PER_SEC);

    // 1.
    time = std::clock();
    ans = 0;
#pragma omp parallel default(none) shared(ans, n, a, b)
    {
#pragma omp critical
        ans += trap(a, b, n); // x
    }
    print(ans);
    printf("%lfs\n", (float) (clock() - time) / CLOCKS_PER_SEC);


    // 2.
    time = std::clock();
    ans = 0;
    double _ans;
#pragma omp parallel default(none) shared(ans, n, a, b) private(_ans)
    {
        _ans = trap(a, b, n);
#pragma omp critical
        ans += _ans;
    }
    print(ans);
    printf("%lfs\n", (float) (clock() - time) / CLOCKS_PER_SEC);


    // 3.
    time = std::clock();
    ans = 0;
#pragma omp parallel default(none) reduction(+: ans) shared(n, a, b)
    {
        ans += trap(a, b, n);
    }
    print(ans);
    printf("%lfs\n", (float) (clock() - time) / CLOCKS_PER_SEC);


    // 4.
    time = std::clock();
    double h = (b - a) / n;
    int i;
    ans = (f(b) - f(a)) / 2;
#pragma omp parallel for default(none) reduction(+: ans) shared(n, a, h)
    for (i = 0; i <= n - 1; i++) {
        ans += f(a + i * h);
    }
    ans *= h;
    print(ans);
    printf("%lfs\n", (float) (clock() - time) / CLOCKS_PER_SEC);

}

double f(double a) {
    return a * a * a;
}

double trap(
    double a, /* in */
    double b,  /* in */
    LL n  /* in */
) {
    double integral, _a, _b, _n, h;
    int k;

    h = (b - a) / n;
    _n = (int) (n / omp_get_num_threads());
    _a = a + omp_get_thread_num() * _n * h;
    _b = _a + _n * h;
    integral = (f(_a) + f(_b)) / 2;
    for (k = 1; k <= _n - 1; k++) {
        integral += f(_a + k * h);
    }
    integral *= h;
    return integral;
}

void lab_1_3() {
    time_t time;
    auto print = [](int arr[], int n) {
        // for (int i = 0; i < n; i++) {
        //     printf("%d ", arr[i]);
        // }
        // printf("\n");
    };
    int n = 20000;
    int arr[n];
    for (int i = 0; i < n; i++) arr[i] = i;
    std::shuffle(arr, arr + n, std::mt19937(std::random_device()()));
    print(arr, n);

    // 0.
    time = std::clock();
    odd_even_sort(arr, n);
    print(arr, n);
    printf("%lfs\n", (float) (clock() - time) / CLOCKS_PER_SEC);

    // 1.
    time = std::clock();
    std::shuffle(arr, arr + n, std::mt19937(std::random_device()()));
    int phase, i;
    for (phase = 0; phase < n; phase++) {
        if (phase % 2 == 0) { // even
#pragma omp parallel for default(none) shared(arr, n)
            for (i = 1; i < n; i += 2) {
                if (arr[i - 1] > arr[i]) {
                    std::swap(arr[i - 1], arr[i]);
                }
            }
        } else {
#pragma omp parallel for default(none) shared(arr, n)
            for (i = 1; i < n - 1; i += 2) {
                if (arr[i] > arr[i + 1]) {
                    std::swap(arr[i], arr[i + 1]);
                }
            }
        }
    }
    print(arr, n);
    printf("%lfs\n", (float) (clock() - time) / CLOCKS_PER_SEC);

    // 2.
    time = std::clock();
    std::shuffle(arr, arr + n, std::mt19937(std::random_device()()));
#pragma omp parallel for default(none) shared(n, arr)
    for (phase = 0; phase < n; phase++) {
        if (phase % 2 == 0) { // even
#pragma omp parallel for default(none) shared(arr, n)
            for (i = 1; i < n; i += 2) {
                if (arr[i - 1] > arr[i]) {
                    std::swap(arr[i - 1], arr[i]);
                }
            }
        } else {
#pragma omp parallel for default(none) shared(arr, n)
            for (i = 1; i < n - 1; i += 2) {
                if (arr[i] > arr[i + 1]) {
                    std::swap(arr[i], arr[i + 1]);
                }
            }
        }
    }
    print(arr, n);
    printf("%lfs\n", (float) (clock() - time) / CLOCKS_PER_SEC);
}

void odd_even_sort(
    int a[], /* in */
    int n /* in */
) {
    int phase, i;
    for (phase = 0; phase < n; phase++) {
        if (phase % 2 == 0) { // even
            for (i = 1; i < n; i += 2) {
                if (a[i - 1] > a[i]) {
                    std::swap(a[i - 1], a[i]);
                }
            }
        } else {
            for (i = 1; i < n - 1; i += 2) {
                if (a[i] > a[i + 1]) {
                    std::swap(a[i], a[i + 1]);
                }
            }
        }
    }
}

#pragma clang diagnostic pop

/*
0.001000s
0.899000s
0.124000s
0.001000s

3.374000s
3.107000s
0.770000s
0.855000s
0.799000s

5.001000s
4.970000s
2.779000s
*/
