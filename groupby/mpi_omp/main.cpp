#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <omp.h>
#include <mpi.h>

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

void with_mpi();

int my_rank, comm_sz, thread_nums;

const int UINT16_MAX_1 = UINT16_MAX + 1;
char **data;
uint16_t **line_bits;
int bits_length;

int main() {
    MPI_Init(nullptr, nullptr);

    omp_set_num_threads(8);
#pragma omp parallel default(none) shared(thread_nums)
    {
#pragma omp single
        thread_nums = omp_get_num_threads();
    }

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    double start = MPI_Wtime();
    with_mpi();
    double end = MPI_Wtime();
    double time_span = end - start;
    if (my_rank == 0) {
        printf("mpi with omp: %lfs\n", time_span);
    }

    MPI_Finalize();
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

void with_mpi() {
    read_file();

    // 1. 基数排序

    // 辅助数组
    auto indies = new int[MAX_LINES];
    for (int i = 0; i < MAX_LINES; i++) {
        indies[i] = i;
    }
    auto indies_tmp = new int *[comm_sz];
    for (int i = 0; i < comm_sz; i++) {
        indies_tmp[i] = new int[MAX_LINES];
    }

    auto cnt = new int *[comm_sz + 1]; // p + 1
    for (int i = 0; i <= comm_sz; i++) {
        cnt[i] = new int[UINT16_MAX_1];
    }
    auto cnt_ts = new int *[thread_nums]; // th
    for (int i = 0; i < thread_nums; i++) {
        cnt_ts[i] = new int[UINT16_MAX_1];
    }

    // 拆分进程
    int _len = ceil((double) MAX_LINES / comm_sz);
    int _from = _len * my_rank;
    int _to = std::min(MAX_LINES, _from + _len);

    // 按照 int16 分组
    uint16_t num;
    for (int part = bits_length - 1; part >= 0; part--) { // 每个数据计数
        // 初始化计数数组
        if (my_rank == 0) {
            memset(cnt[0], 0, UINT16_MAX_1 * sizeof(int));
            memset(cnt[comm_sz], 0, UINT16_MAX_1 * sizeof(int));
        } else {
            memset(cnt[my_rank], 0, UINT16_MAX_1 * sizeof(int));
        }
        for (int i = 0; i < thread_nums; i++) {
            memset(cnt_ts[i], 0, UINT16_MAX_1 * sizeof(int));
        }
        // 同步 indies 辅助数组
        MPI_Bcast(indies, MAX_LINES, MPI_INT, 0, MPI_COMM_WORLD);

        // 顺序计数
#pragma omp parallel for default(none) shared(_from, _to, line_bits, indies, part, cnt_ts) private(num)
        for (int i = _from; i < _to; i++) {
            int my_rank_p = omp_get_thread_num();
            num = line_bits[indies[i]][part];
            cnt_ts[my_rank_p][num]++;
        }
        for (int i = 0; i < thread_nums; i++) { // 整合线程结果
            for (int j = 0; j < UINT16_MAX_1; j++) {
                cnt[my_rank][j] += cnt_ts[i][j];
            }
        }

        // 合并 cnt 处理前缀和
        if (my_rank != 0) {
            MPI_Send(cnt[my_rank], UINT16_MAX_1, MPI_INT, 0, 1, MPI_COMM_WORLD);
            MPI_Recv(cnt[my_rank], UINT16_MAX_1, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        } else {
            // 收集 cnt 数组
            for (int i = 1; i < comm_sz; i++) {
                MPI_Recv(cnt[i], UINT16_MAX_1, MPI_INT, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }

            for (int i = 0; i < comm_sz; i++) {
                for (int j = 0; j < UINT16_MAX_1; j++) {
                    cnt[comm_sz][j] += cnt[i][j];
                }
            }
            for (int i = 1; i < UINT16_MAX_1; i++) {
                cnt[comm_sz][i] += cnt[comm_sz][i - 1];
            }
            for (int i = comm_sz - 1; i >= 0; i--) {
                for (int j = 0; j < UINT16_MAX_1; j++) {
                    cnt[i][j] = cnt[i + 1][j] - cnt[i][j]; // 各个进程对应低位取值为 0 ~ 65535 的元素的存放位置
                }
            }

            // 分发 cnt 数组
            for (int i = 1; i < comm_sz; i++) {
                MPI_Send(cnt[i], UINT16_MAX_1, MPI_INT, i, 2, MPI_COMM_WORLD);
            }
        }

        // 调整下标数组
        memset(indies_tmp[my_rank], -1, MAX_LINES * sizeof(int));
        for (int i = _from; i < _to; i++) { // 从前往后，cnt 中存储的是从此处开始存
            num = line_bits[indies[i]][part];
            indies_tmp[my_rank][cnt[my_rank][num]++] = indies[i];
        }
        // 整合下标数组
        if (my_rank != 0) {
            MPI_Send(indies_tmp[my_rank], MAX_LINES, MPI_INT, 0, 3, MPI_COMM_WORLD);
        } else {
#pragma omp parallel for default(none) shared(indies_tmp, indies)
            for (int i = 0; i < MAX_LINES; i++) {
                if (indies_tmp[0][i] != -1) {
                    indies[i] = indies_tmp[0][i];
                }
            }
            for (int i = 1; i < comm_sz; i++) {
                MPI_Recv(indies_tmp[i], MAX_LINES, MPI_INT, i, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
#pragma omp parallel for default(none) shared(indies_tmp, indies, i)
                for (int j = 0; j < MAX_LINES; j++) {
                    if (indies_tmp[i][j] != -1) {
                        indies[j] = indies_tmp[i][j];
                    }
                }
            }
        }
    }

    // 2. 分配分组号
    auto flags = new int[MAX_LINES];
    if (my_rank == 0) {
        memset(flags, 0, MAX_LINES * sizeof(int));
        for (int i = 1; i < MAX_LINES; i++) {
            if (strcmp(data[indies[i]], data[indies[i - 1]]) == 0) {
                flags[i] = flags[i - 1];
            } else {
                flags[i] = flags[i - 1] + 1;
            }
        }
    }

    // 3. 输出
    if (my_rank == 0) {
        for (int i = 0; i < 10; i++) {
            printf("%s\n", data[indies[i]]);
        }
        printf("(Group number: %d)\n", flags[MAX_LINES - 1]);
    }

    delete[] flags;
    for (int i = 0; i < thread_nums; i++) {
        delete[] cnt_ts[i];
    }
    delete[] cnt_ts;
    for (int i = 0; i < comm_sz + 1; i++) {
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
