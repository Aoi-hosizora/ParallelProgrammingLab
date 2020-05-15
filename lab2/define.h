#pragma once

namespace N1 {
    void _mtx_vec_mul(
        const double *mtx, /* in */
        const double *vec, /* in */
        double *res,  /* out */
        int n, /* in */
        int m /* in */
    );

    void mtx_vec_mul(
        const double *mtx, /* in */
        const double *vec, /* in */
        double *res,  /* out */
        int n, /* in */
        int m /* in */
    );
}

namespace N2 {
    double _trap(double a, double b, int n);

    double trap(double a, double b, int n);
}

namespace N3 {
    void shuffle(int *arr, int n);

    void _odd_even_sort(int *arr, int n);

    void odd_even_sort(int *arr, int n);
}