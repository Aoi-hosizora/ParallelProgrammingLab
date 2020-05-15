#include <algorithm>
#include <random>
#include "define.h"

void N3::shuffle(int *arr, int n) {
    std::shuffle(arr, arr + n, std::mt19937(std::random_device()()));
}

void N3::_odd_even_sort(int *arr, int n) {
    int phase, i;
    for (phase = 0; phase < n; phase++) {
        if (phase % 2 == 0) { // even
            for (i = 1; i < n; i += 2) {
                if (arr[i - 1] > arr[i]) {
                    std::swap(arr[i - 1], arr[i]);
                }
            }
        } else {
            for (i = 1; i < n - 1; i += 2) {
                if (arr[i] > arr[i + 1]) {
                    std::swap(arr[i], arr[i + 1]);
                }
            }
        }
    }

    // for (i = 0; i < n; i++) {
    //     printf("%d ", arr[i]);
    // }
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-branch-clone"

void N3::odd_even_sort(int *arr, int n) {
    int phase, i;

#pragma omp parallel for default(none) shared(n, arr) private(i)
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

    // for (i = 0; i < n; i++) {
    //     printf("%d ", arr[i]);
    // }
}

#pragma clang diagnostic pop
