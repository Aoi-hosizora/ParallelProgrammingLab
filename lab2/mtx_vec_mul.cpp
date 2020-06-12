#include <mpi.h>
#include "define.h"
#include <cstdio>
#include <cstring>
#include <cmath>
#include <algorithm>

void N1::_mtx_vec_mul(
    const double *mtx, /* in */
    const double *vec, /* in */
    double *res,  /* out */
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

    for (i = 0; i < 5; i++) {
        printf("%lf ", res[i]);
    }
    printf("\n");
}

void N1::mtx_vec_mul(
    const double *mtx, /* in */ // n * m
    const double *vec, /* in */ // m * 1
    double *res,  /* out */
    int n, /* in */
    int m /* in */
) {
    int comm_sz, my_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    int _length = ceil((double) n / comm_sz);
    int _start = my_rank * _length;
    int _end = std::min(_start + _length, n);

    // 根据当前的 rank 拆分每个进程的计算
    auto _res = new double[n];
    memset(_res, -1, n * sizeof(double));
    for (int r = _start; r < _end; r++) {
        _res[r] = 0;
        for (int c = 0; c < m; c++) {
            _res[r] += mtx[r * m + c] * vec[c];
        }
    }
    // 当前如果是非 0 进程，将计算结果传给 0 进程处理
    if (my_rank != 0) {
        MPI_Send(_res, n, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    } else {
        memset(res, 0, n * sizeof(double));
        // 对每个进程的计算结果叠加
        for (int i = _start; i < _end; i++) {
            res[i] = _res[i];
        }
        for (int i = 1; i < comm_sz; i++) {
            MPI_Recv(_res, n, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            int is = i * _length;
            int ie = std::min(is + _length, n);
            for (int j = is; j < ie; j++) {
                res[j] = _res[j];
            }
        }
    }

    delete[] _res;

    if (my_rank == 0) {
        for (int i = 0; i < 5; i++) {
            printf("%lf ", res[i]);
        }
        printf("\n");
    }
}
