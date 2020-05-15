#include <mpi.h>
#include "define.h"
#include <cstdio>

void N1::_mtx_vec_mul(
    const double *mtx, /* in */
    const double *vec, /* in */
    double *res,  /* out */
    int n, /* in */
    int m /* in */
) {
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    if (my_rank != 0) return;

    int i, j;
    for (i = 0; i < n; i++) {
        res[i] = 0;
        for (j = 0; j < m; j++) {
            res[i] += mtx[i * m + j] * vec[j];
        }
    }

    // for (i = 0; i < n; i++)
    //     printf("%lf ", res[i]);
    // printf("\n");
}

void N1::mtx_vec_mul(
    const double *mtx, /* in */
    const double *vec, /* in */
    double *res,  /* out */
    int n, /* in */
    int m /* in */
) {
    int comm_sz, my_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    double _res[n];
    for (int r = my_rank; r < n; r += comm_sz) {
        _res[r] = 0;
        for (int c = 0; c < m; c++) {
            _res[r] += mtx[r * m + c] * vec[c];
        }
    }

    if (my_rank != 0) {
        MPI_Send(_res, n, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    } else {
        for (int i = 0; i < n; i++) {
            res[i] = _res[i];
        }
        for (int i = 1; i < comm_sz; i++) {
            MPI_Recv(_res, n, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int j = 0; j < n; j++) {
                res[j] += _res[j];
            }
        }

        // for (int i = 0; i < n; i++)
        //     printf("%lf ", res[i]);
        // printf("\n");
    }
}
