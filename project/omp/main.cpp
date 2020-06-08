#include <cstdio>
#include <omp.h>
#include <cstdlib>
#include <cstring>
#include <cstdint>

// a-g 1000 2-4
const int MAX_LINE_LENGTH = 6;
const char PATH[] = "G:/parallel/1K.txt";
const int MAX_LINES = 1000;

// a-g 10000 5-10
// const int MAX_LINE_LENGTH = 12;
// const char PATH[] = "G:/parallel/10K.txt";
// const int MAX_LINES = 10000;

// 0-9a-gA-G 100000 10-20
// const int MAX_LINE_LENGTH = 22;
// const char PATH[] = "G:/parallel/100K.txt";
// const int MAX_LINES = 100000;

// 0-9a-gA-G 1000000 10-20
// const int MAX_LINE_LENGTH = 22;
// const char PATH[] = "G:/parallel/1M.txt";
// const int MAX_LINES = 1000000;

// 0-9a-zA-Z 10000000 10-20
// const int MAX_LINE_LENGTH = 22;
// const char PATH[] = "G:/parallel/10M.txt";
// const int MAX_LINES = 10000000;

// 0-9a-zA-Z 100000000 10-50
// const int MAX_LINE_LENGTH = 52;
// const char PATH[] = "G:/parallel/100M.txt";
// const int MAX_LINES = 100000000;

// 0-9a-zA-Z 1000000000 10-50
// const int MAX_LINE_LENGTH = 52;
// const char PATH[] = "G:/parallel/1G.txt";
// const int MAX_LINES = 1000000000;

void read_file();

void without_omp();

void with_omp();

int main() {
    omp_set_num_threads(8);
    int thread_nums;
#pragma omp parallel default(none) shared(thread_nums)
    {
#pragma omp single
        thread_nums = omp_get_num_threads();
    }

    // sequential
    double seq_start = omp_get_wtime();
    without_omp();
    double seq_time = omp_get_wtime() - seq_start;
    printf("sequence: %lfs\n", seq_time);

    // parallel
    double par_start = omp_get_wtime();
    with_omp();
    double par_time = omp_get_wtime() - par_start;
    printf("parallel: %lfs\n", par_time);

    // speedup
    double speedup = seq_time / par_time;
    printf("speedup: %lf\n", speedup);

    // efficiency
    double efficiency = speedup / thread_nums;
    printf("efficiency: %lf\n", efficiency);
}

char **data;
int *indies; // 下标辅助数组
int *indies_tmp; // 下标辅助数组，调整临时用
int *cnt; // 计数数组

void read_file() {
    auto *f = fopen(PATH, "r");
    if (f == nullptr) {
        printf("File not found");
        exit(2);
    }
    data = new char *[MAX_LINES]; // <<< new
    for (int i = 0; i < MAX_LINES; i++) {
        data[i] = new char[MAX_LINE_LENGTH];
        memset(data[i], 0, MAX_LINE_LENGTH * sizeof(char));
    }

    int l = 0;
    while (fgets(data[l], MAX_LINE_LENGTH, f) != nullptr) {
        int i = 0;
        while (data[l][i] != '\0') {
            if (data[l][i] == '\n' || data[l][i] == '\r') {
                data[l][i] = '\0';
                break;
            }
            i++;
        }
        l++;
    }
    fclose(f);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "hicpp-signed-bitwise"

/**
* 2 Chars to 1 Uint16 (0-65535)
* @param line data[i]
* @param j MAX_LINE_LENGTH - 2 -> 0
*/
uint16_t chars_to_uint16(const char *line, int j) {
    uint16_t num = 0;
    int len = strlen(line);
    int idx = j - (MAX_LINE_LENGTH - len);
    if (idx >= 0) {
        num = (line[idx] << 8) + line[idx + 1];
        // num = *(uint16_t *) &line[indies[idx]];
    } else if (idx + 1 >= 0) {
        num = line[idx + 1];
    }
    return num;
}

void without_omp() {
    read_file();

    // 1. 基数排序

    // 辅助数组
    indies = new int[MAX_LINES];
    indies_tmp = new int[MAX_LINES];
    for (int i = 0; i < MAX_LINES; i++) {
        indies[i] = i;
    }
    cnt = new int[UINT16_MAX + 1];

    // 按照 int16 分组
    uint16_t num;
    for (int j = MAX_LINE_LENGTH - 2; j >= 0; j -= 2) {
        // 每个数据计数
        memset(cnt, 0, (UINT16_MAX + 1) * sizeof(int));
        for (int i = 0; i < MAX_LINES; i++) {
            num = chars_to_uint16(data[indies[i]], j);
            cnt[num]++;
        }
        // 前缀和
        for (int n = 1; n <= UINT16_MAX; n++) {
            cnt[n] += cnt[n - 1];
        }
        // 调整下标数组
        memcpy(indies_tmp, indies, MAX_LINES * sizeof(int));
        for (int i = MAX_LINES - 1; i >= 0; i--) {
            num = chars_to_uint16(data[indies[i]], j);
            indies_tmp[--cnt[num]] = indies[i];
        }
        memcpy(indies, indies_tmp, MAX_LINES * sizeof(int));
    }

    // 2. 分配分组号

    // 标志
    auto flags = new int[MAX_LINES];
    memset(flags, 0, MAX_LINES * sizeof(int));
    for (int i = 1; i < MAX_LINES; i++) {
        if (strcmp(data[indies[i]], data[indies[i - 1]]) != 0) {
            flags[i] = 1;
        }
    }
    // 子集前缀和
    for (int i = 1; i < MAX_LINES; i++) {
        if (strcmp(data[indies[i]], data[indies[i - 1]]) == 0) {
            flags[i] += flags[i - 1];
        }
    }
    // 分组号
    for (int i = 1; i < MAX_LINES; i++) {
        if (strcmp(data[indies[i]], data[indies[i - 1]]) == 0) {
            flags[i] = flags[i - 1];
        } else {
            flags[i] = flags[i - 1] + 1;
        }
    }

    // 3. 输出
    // for (int i = 0; i < MAX_LINES; i++) {
    //     printf("%s %d\n", data[indies[i]], flags[i]);
    // }
    printf("(Group number: %d)\n", flags[MAX_LINES - 1]);

    delete[] flags;
    delete[] cnt;
    delete[] indies_tmp;
    delete[] indies;
    for (int i = 0; i < MAX_LINES; i++) {
        delete[] data[i];
    }
    delete[] data;
}

void with_omp() {
    read_file();
    for (int i = 0; i < MAX_LINES; i++) {
        delete[] data[i];
    }
    delete[] data;
}

#pragma clang diagnostic pop
