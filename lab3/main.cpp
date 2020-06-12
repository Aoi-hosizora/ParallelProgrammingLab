#include <cstdio>
#include <cmath>
#include <algorithm>
#include <random>
#include <ctime>
#include <mpi.h>
#include <omp.h>
#include <cstring>

#pragma ide diagnostic ignored "modernize-use-auto"
#pragma ide diagnostic ignored "modernize-use-nullptr"

int comm_sz, my_rank;

void without_mpi();

void with_mpi();

int n = 100000, m = 100;
double *mtx;
double *vec;

int main() {
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    omp_set_num_threads(8);

    // data
    mtx = new double[n * m]; // n * m
    vec = new double[m]; // m * 1
    if (my_rank == 0) {
        for (int i = 0; i < n * m; i++) mtx[i] = 20 + i % 80;
        for (int i = 0; i < m; i++) vec[i] = 20 + i % 80;
        unsigned seed = time(0);
        std::shuffle(mtx, mtx + n * m, std::default_random_engine(seed));
        std::shuffle(vec, vec + m, std::default_random_engine(seed));
    }

    // sequential
    double seq_start, seq_end, seq_timespan;
    if (my_rank == 0) {
        seq_start = MPI_Wtime();
        without_mpi();
        seq_end = MPI_Wtime();
        seq_timespan = seq_end - seq_start;
        printf("sequential: %lfs\n", seq_timespan);
    }

    // parallel
    double par_start = MPI_Wtime();
    with_mpi();
    double par_end = MPI_Wtime();
    double par_timespan = par_end - par_start;
    if (my_rank == 0) {
        printf("parallel: %lfs\n", par_timespan);
    }

    if (my_rank == 0) {
        double speedup = seq_timespan / par_timespan;
        printf("speedup: %lf\n", speedup);
        double efficiency = speedup / comm_sz;
        printf("efficiency: %lf\n", efficiency);
    }

    delete[] mtx;
    delete[] vec;

    MPI_Finalize();
}

void without_mpi() {
    double *res = new double[n]; // n * 1

    for (int i = 0; i < n; i++) {
        res[i] = 0;
        for (int j = 0; j < m; j++) {
            res[i] += mtx[i * m + j] * vec[j];
        }
    }

    for (int i = 0; i < 5; i++) {
        printf("%lf ", res[i]);
    }
    printf("\n");

    delete[] res;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedValue"

void with_mpi() {
    int _length = ceil((double) n / comm_sz);
    int _start = my_rank * _length;
    int _end = std::min(_start + _length, n);

    int _row_size = _end - _start;
    double *_mtx = new double[_row_size * m];

    if (my_rank == 0) {
        memcpy(_mtx, mtx, _length * sizeof(double));
        for (int i = 1; i < comm_sz; i++) {
            int is = i * _length;
            int ie = std::min(is + _length, n);
            int irs = ie - is;
            MPI_Send(mtx + is * m, irs * m, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
        }
    } else {
        MPI_Recv(_mtx, _row_size * m, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    MPI_Bcast(vec, m, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    double *_res = new double[_row_size]; // n * 1
    memset(_res, 0, _row_size * sizeof(double));

#pragma omp parallel for default(none) shared(_row_size, _res, _mtx, vec, m)
    for (int r = 0; r < _row_size; r++) {
        for (int c = 0; c < m; c++) {
            _res[r] += _mtx[r * m + c] * vec[c];
        }
    }

    double *res = new double[n];
    memset(res, 0, n * sizeof(double));
    if (my_rank != 0) {
        MPI_Send(_res, _row_size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    } else {
        memcpy(res, _res, _row_size * sizeof(double));
        for (int i = 1; i < comm_sz; i++) {
            MPI_Recv(_res, _row_size, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            int is = i * _length;
            int ie = std::min(is + _length, n);
            int irs = ie - is;
#pragma omp parallel for default(none) shared(is, irs, _res, res)
            for (int j = 0; j < irs; j++) {
                res[j + is] = _res[j];
            }
        }
    }

    if (my_rank == 0) {
        for (int i = 0; i < 5; i++) {
            printf("%lf ", res[i]);
        }
        printf("\n");
    }

    delete[] _mtx;
    delete[] _res;
    delete[] res;
}

#pragma clang diagnostic pop
