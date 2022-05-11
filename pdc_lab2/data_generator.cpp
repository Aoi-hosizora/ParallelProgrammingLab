#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include "predefined.h"

inline float rand_float(float s) {
    return 4 * s * (1 - s);
}

void matrix_gen(float *a, uint64_t n, float seed) {
    float s = seed;
    for (uint64_t i = 0; i < n; i++) {
        do {
            s = rand_float(s);
        } while (s < 1e-8);
        a[i] = s;
    }
}

int main() {
    float *data = (float *) malloc(N * sizeof(float));
    if (data == nullptr) {
        printf("Failed to allocate memory.\n");
        exit(-1);
    }

    // generate data
    printf("Generating...\n");
    srand(time(0));
    float seed = (rand() % (int) 1e8) / 1e8;
    matrix_gen(data, N, seed);

    // save data to file
    printf("Saving...\n");
    auto f = fopen(FILENAME, "wb");
    if (f == nullptr) {
        printf("Failed to create file.\n");
        exit(-1);
    }
    for (int i = 0; i < N; i++) {
        fwrite(data + i, sizeof(float), 1, f);
    }
    fclose(f);

    // qsort
    printf("Sorting...\n");
    auto start_time = std::clock();
    std::qsort(data, N, sizeof(float), [](const void *a, const void *b) -> int {
        float fa = *(float *) a, fb = *(float *) b;
        return (fa > fb) - (fa < fb);
    });
    auto end_time = std::clock();
    auto duration = (float) (end_time - start_time) / CLOCKS_PER_SEC;
    printf("Duration: %.3fs\n", duration);

    // check if sorted result is in ascending order
    bool in_order = true;
    for (int i = 1; i < N; i++) {
        if (data[i - 1] > data[i]) {
            printf("Sorted result are not in ascending order !!!\n");
            break;
        }
    }
    // setbuf(stdout, NULL);
    // for (int i = 0; i < 2000; i++) {
    //     printf("%.20f\n", data[i * 5000]);
    // }

    // release memories
    free(data);
}
