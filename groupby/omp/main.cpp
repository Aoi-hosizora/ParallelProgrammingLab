#include <cstdio>
#include <omp.h>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cstdint>

// a-g 1000 2-4
// const int MAX_LINE_LENGTH = 6;
// const char PATH[] = "G:/parallel/1K.txt";
// const int MAX_LINES = 1000;

// a-g 10000 5-10
// const int MAX_LINE_LENGTH = 12;
// const char PATH[] = "G:/parallel/10K.txt";
// const int MAX_LINES = 10000;

// 0-9a-gA-G 100000 10-20
// const int MAX_LINE_LENGTH = 22;
// const char PATH[] = "G:/parallel/100K.txt";
// const int MAX_LINES = 100000;

// 数据集1
// 0-9a-gA-G 1000000 10-20
// const int MAX_LINE_LENGTH = 22;
// const char PATH[] = "G:/parallel/1M.txt";
// const int MAX_LINES = 1000000;

// 数据集2
// 0-9a-zA-Z 10000000 10-20
const int MAX_LINE_LENGTH = 22;
const char PATH[] = "G:/parallel/10M.txt";
const int MAX_LINES = 10000000;

// 数据集3
// 0-9a-zA-Z 20000000 10-20
// const int MAX_LINE_LENGTH = 22;
// const char PATH[] = "G:/parallel/20M.txt";
// const int MAX_LINES = 20000000;

// 数据集4
// 0-9a-zA-Z 5000000 20-100
// const int MAX_LINE_LENGTH = 102;
// const char PATH[] = "G:/parallel/5M.txt";
// const int MAX_LINES = 5000000;

// 0-9a-zA-Z 100000000 10-50
// const int MAX_LINE_LENGTH = 52;
// const char PATH[] = "G:/parallel/100M.txt";
// const int MAX_LINES = 100000000;

// 0-9a-zA-Z 1000000000 10-50
// const int MAX_LINE_LENGTH = 52;
// const char PATH[] = "G:/parallel/1G.txt";
// const int MAX_LINES = 1000000000;

uint16_t chars_to_uint16(const char *, int);

void read_file();

void with_omp();

int thread_nums;

const int UINT16_MAX_1 = UINT16_MAX + 1;
char **data;
uint16_t **line_bits;
int bits_length;

int main() {
    omp_set_num_threads(16);
#pragma omp parallel default(none) shared(thread_nums)
    {
#pragma omp single
        thread_nums = omp_get_num_threads();
    }

    double start = omp_get_wtime();
    with_omp();
    double end = omp_get_wtime();
    double time_span = end - start;
    printf("omp: %lfs\n", time_span);
}

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

    // char to int16
    line_bits = new uint16_t *[MAX_LINES];
    bits_length = ceil(MAX_LINE_LENGTH / 2.0);
    for (int i = 0; i < MAX_LINES; i++) {
        line_bits[i] = new uint16_t[bits_length];
        for (int j = bits_length - 1; j >= 0; j--) {
            line_bits[i][j] = chars_to_uint16(data[i], j * 2);
        }
    }
}

uint16_t chars_to_uint16(const char *line, int part) {
#pragma clang diagnostic push
#pragma ide diagnostic ignored "hicpp-signed-bitwise"

    uint16_t num = 0;
    int len = strlen(line);
    int idx = part - (MAX_LINE_LENGTH - len);
    if (idx >= 0) {
        num = (line[idx] << 8) + line[idx + 1];
        // num = *(uint16_t *) &line[indies[idx]];
    } else if (idx + 1 >= 0) {
        num = line[idx + 1];
    }
    return num;

#pragma clang diagnostic pop
}

void with_omp() {
    read_file();

    // 1. 基数排序

    // 辅助数组
    auto indies = new int[MAX_LINES];
    auto indies_tmp = new int[MAX_LINES];
    for (int i = 0; i < MAX_LINES; i++) {
        indies[i] = i;
    }

    auto cnt = new int *[thread_nums + 1]; // p + 1
    for (int i = 0; i <= thread_nums; i++) {
        cnt[i] = new int[UINT16_MAX_1];
    }

    // 按照 int16 分组
    uint16_t num;
    int my_rank = 0;
    for (int part = bits_length - 1; part >= 0; part--) {
        // 分线程计数
        for (int i = 0; i < thread_nums + 1; i++) {
            memset(cnt[i], 0, UINT16_MAX_1 * sizeof(int));
        }
#pragma omp parallel for default(none) shared(part, line_bits, indies, cnt) private(my_rank, num)
        for (int i = 0; i < MAX_LINES; i++) {
            my_rank = omp_get_thread_num();
            num = line_bits[indies[i]][part];
            cnt[my_rank][num]++;
        }

        // 合并 cnt 处理前缀和
        for (int i = 0; i < thread_nums; i++) {
            for (int j = 0; j < UINT16_MAX_1; j++) {
                cnt[thread_nums][j] += cnt[i][j];
            }
        }
        for (int i = 1; i < UINT16_MAX_1; i++) {
            cnt[thread_nums][i] += cnt[thread_nums][i - 1];
        }
        for (int i = thread_nums - 1; i >= 0; i--) {
            for (int j = 0; j < UINT16_MAX_1; j++) {
                cnt[i][j] = cnt[i + 1][j] - cnt[i][j]; // 各个线程对应低位取值为 0 ~ 65535 的元素的存放位置
            }
        }

        // 重新排放辅助数组
#pragma omp parallel for default(none) shared(part, line_bits, indies, indies_tmp, cnt) private(num, my_rank)
        for (int i = 0; i < MAX_LINES; i++) { // 从前往后，cnt 中存储的是从此处开始存
            my_rank = omp_get_thread_num();
            num = line_bits[indies[i]][part];
            indies_tmp[cnt[my_rank][num]++] = indies[i];
        }
        memcpy(indies, indies_tmp, MAX_LINES * sizeof(int));
    }

    // 2. 分配分组号
    auto flags = new int[MAX_LINES];
    memset(flags, 0, MAX_LINES * sizeof(int));
    for (int i = 1; i < MAX_LINES; i++) {
        if (strcmp(data[indies[i]], data[indies[i - 1]]) == 0) {
            flags[i] = flags[i - 1];
        } else {
            flags[i] = flags[i - 1] + 1;
        }
    }

    // 3. 输出
    for (int i = 0; i < 10; i++) {
        printf("%s\n", data[indies[i]]);
    }
    printf("(Group number: %d)\n", flags[MAX_LINES - 1]);

    delete[] flags;
    for (int i = 0; i < thread_nums + 1; i++) {
        delete[] cnt[i];
    }
    delete[] cnt;
    delete[] indies_tmp;
    delete[] indies;
    for (int i = 0; i < bits_length; i++) {
        delete[] line_bits[i];
    }
    delete[] line_bits;
    for (int i = 0; i < MAX_LINES; i++) {
        delete[] data[i];
    }
    delete[] data;
}
