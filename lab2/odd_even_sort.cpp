#include <algorithm>
#include <random>
#include <cmath>
#include <mpi.h>
#include <cstring>
#include "define.h"

void N3::shuffle(int *arr, int n) {
    std::shuffle(arr, arr + n, std::mt19937(std::random_device()()));
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "hicpp-signed-bitwise"

void N3::_odd_even_sort(int *arr, int n) {
    int i;
    for (int phase = 0; phase < n; phase++) {
        if (phase & 1) {
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

    for (i = 0; i < 20; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

void N3::odd_even_sort(int *arr, int n) {
    // 先利用 Bcast 将数据广播给每个进程
    MPI_Bcast(arr, n, MPI_INT, 0, MPI_COMM_WORLD);

    int comm_sz, my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    // 根据当前的 rank 拆分数据集
    int _len = ceil((double) n / comm_sz);
    int _start = _len * my_rank;
    int _end = std::min(n, _start + _len);

    // 每个进程的排序
    int i;
    for (int phase = 0; phase < _end - _start; phase++) {
        if (phase & 1) {
            for (i = _start + 1; i < _end; i += 2) {
                if (arr[i - 1] > arr[i]) {
                    std::swap(arr[i - 1], arr[i]);
                }
            }
        } else {
            for (i = _start + 1; i < _end - 1; i += 2) {
                if (arr[i] > arr[i + 1]) {
                    std::swap(arr[i], arr[i + 1]);
                }
            }
        }
    }

    // 当前如果是非 0 进程，将排序结果传给 0 进程合并
    if (my_rank != 0) {
        MPI_Send(arr, n, MPI_INT, 0, 0, MPI_COMM_WORLD);
    } else {
        auto merge = new int *[comm_sz];
        for (i = 0; i < comm_sz; i++) {
            merge[i] = new int[n];
        }
        int starts[comm_sz];
        int ends[comm_sz];

        memcpy(merge[0], arr, n * sizeof(int));
        starts[0] = _start;
        ends[0] = _end;

        // 收集各个进程的排序结果
        for (i = 1; i < comm_sz; i++) {
            MPI_Recv(merge[i], n, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            starts[i] = _len * i;
            ends[i] = std::min(n, starts[i] + _len);
        }

        // 归并 O(an)
        for (i = 0; i < n; i++) {
            int min_p = 0;
            int min_data = 0x3f3f3f3f;
            for (int p = 0; p < comm_sz; p++) {
                if (starts[p] < ends[p] && merge[p][starts[p]] < min_data) {
                    min_p = p;
                    min_data = merge[p][starts[p]];
                }
            }
            arr[i] = merge[min_p][starts[min_p]];
            starts[min_p]++;
        }
    }

    if (my_rank == 0) {
        for (i = 0; i < 20; i++) {
            printf("%d ", arr[i]);
        }
        printf("\n");
    }
}

#pragma clang diagnostic pop
