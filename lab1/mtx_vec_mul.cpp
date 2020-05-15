#include "define.h"

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

    // printf(" %lf ", *res);
}

void N1::mtx_vec_mul(
    const double *mtx, /* in */
    const double *vec, /* in */
    double *res,  /* out */
    int n, /* in */
    int m /* in */
) {
    int i, j;

#pragma omp parallel for default(none) shared(mtx, vec, res, n, m) private(j)
    for (i = 0; i < n; i++) {
        res[i] = 0;
        for (j = 0; j < m; j++) {
            res[i] += mtx[i * m + j] * vec[j];
        }
    }

    // printf(" %lf ", *res);
}
