void mat_vect_mult(
    double A[], /* in */
    double x[], /* in */
    double y[], /* out */
    int m, /* in */
    int n /* in */
) {
    for (int i = 0; i < m; i++) {
        y[i] = 0;
        for (int j = 0; j < n; j++) {
            y[i] += A[i * n + j] * x[j];
        }
    }
}
