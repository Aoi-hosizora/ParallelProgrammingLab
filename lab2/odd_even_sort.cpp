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
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    if (my_rank != 0) return;

    int phase, i;
    for (phase = 0; phase < n; phase++) {
        if (phase & 1) { // even
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
    // printf("\n");
}

void N3::odd_even_sort(int *arr, int n) {
    int comm_sz, my_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    int _n = floor((double) n / comm_sz);
    int tmp_arr[_n];
    memcpy(tmp_arr, arr + my_rank * _n, _n * sizeof(int));

    int phase, i;
    for (phase = 0; phase < _n; phase++) {
        if (phase & 1) { // even
            for (i = 1; i < _n; i += 2) {
                if (tmp_arr[i - 1] > tmp_arr[i]) {
                    std::swap(tmp_arr[i - 1], tmp_arr[i]);
                }
            }
        } else {
            for (i = 1; i < _n - 1; i += 2) {
                if (tmp_arr[i] > tmp_arr[i + 1]) {
                    std::swap(tmp_arr[i], tmp_arr[i + 1]);
                }
            }
        }
    }

    int rcv_arrs[comm_sz][_n];
    MPI_Gather(tmp_arr, _n, MPI_INT, rcv_arrs, _n, MPI_INT, 0, MPI_COMM_WORLD);
    if (my_rank == 0) {
        int tmp_cnt = n - _n * comm_sz;
        int tmp[tmp_cnt];
        memcpy(tmp, arr + _n * comm_sz, sizeof(tmp));
        std::sort(tmp, tmp + tmp_cnt);

        int indies[comm_sz + 1];
        memset(indies, 0, sizeof(indies));
        int dn = 0;
        while (dn < n) {
            int pmin = INT_MAX;
            int pid = -1;
            for (int p = 0; p < comm_sz; p++) {
                if (indies[p] < _n && pmin > rcv_arrs[p][indies[p]]) {
                    pmin = rcv_arrs[p][indies[p]];
                    pid = p;
                }
            }
            if (indies[comm_sz] < tmp_cnt && pmin > tmp[indies[comm_sz]]) {
                pmin = tmp[indies[comm_sz]];
                pid = comm_sz;
            }
            arr[dn++] = pmin;
            indies[pid]++;
        }

        // for (i = 0; i < n; i++) {
        //     printf("%d ", arr[i]);
        // }
        // printf("\n");
    }
}

#pragma clang diagnostic pop
