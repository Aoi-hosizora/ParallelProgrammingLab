#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>

float rand_float(float s) {
    return 4 * s * (1 - s);
}

void matrix_gen(float *a, uint64_t N, float seed) {
    float s = seed;
    for (uint64_t i = 0; i < N; i++) {
        s = rand_float(s);
        a[i] = s;
    }
}

const uint64_t N = (uint64_t) 256 * 1024 * 1024;
const char *FILENAME = "./256M.txt";
// const uint64_t N = (uint64_t) 1024 * 1024 * 1024;
// const char *FILENAME = "./1G.txt";
// const uint64_t N = (uint64_t) 4 * 1024 * 1024 * 1024;
// const char *FILENAME = "./4G.txt";

int main() {
    float *data = (float *) malloc(N * sizeof(float));
    if (data == nullptr) {
        printf("Failed to allocate memory.\n");
        exit(-1);
    }

    printf("Generating...\n");
    srand(time(0));
    float seed = (rand() % (int) 1e8) / 1e8;
    matrix_gen(data, N, seed);

    printf("Saving...\n");
    auto f = fopen(FILENAME, "wb");
    if (f == nullptr) {
        printf("Failed to create file.\n");
        exit(-1);
    }
    fwrite(data, sizeof(float), N, f);
    fclose(f);

    printf("Sorting...\n");
    auto start_time = std::clock();
    std::qsort(data, N, sizeof(float), [](const void *a, const void *b) -> int {
        float fa = *(float *) a, fb = *(float *) b;
        return (fa > fb) - (fa < fb); // <<<
    });
    auto end_time = std::clock();
    auto duration = (float) (end_time - start_time) / CLOCKS_PER_SEC;
    printf("Duration: %.3fs\n", duration);

    setbuf(stdout, NULL);
    for (int i = 0; i < 2000; i++) {
        printf("%.20f\n", data[i * 5000]);
    }

    free(data);
}
