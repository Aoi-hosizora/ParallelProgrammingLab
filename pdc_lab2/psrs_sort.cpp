#include <mpi.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <queue>

#include "predefined.h"

void qsort(float *, uint64_t);

void multiway_merge(float **, int *, int, float *, int);

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    int comm_sz, my_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    int data_length = N / comm_sz;  // TODO uint64_t
    printf("comm_sz: %d, my_rank: %d, data_length: %d\n", comm_sz, my_rank, data_length);

    // MPI_File logger;
    // auto log = [&logger](const char *content) {
    //     MPI_File_write_ordered(logger, content, strlen(content), MPI_CHAR, MPI_STATUS_IGNORE);
    // };
    // MPI_File_open(MPI_COMM_WORLD, "logger.log", MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &logger);

    // 0. read data
    char filepath[MAX_PATH_LEN] = {0};
    sprintf(filepath, FILENAME_FMT, my_rank + 1);
    float *data = (float *) malloc(data_length * sizeof(float));
    if (data == nullptr) {
        printf("Failed to allocate memory.\n");
        exit(-1);
    }
    auto f = fopen(filepath, "rb");
    if (f == nullptr) {
        printf("Failed to open file %s.\n", filepath);
        exit(-1);
    }
    fread(data, sizeof(float), data_length, f);
    fclose(f);

    // 1. sort self
    auto start_time = MPI_Wtime();
    qsort(data, data_length);

    // 2. sample
    float *samples = (float *) malloc(comm_sz * sizeof(float));
    for (int i = 0; i < comm_sz; i++) {
        samples[i] = data[i * (data_length / comm_sz)];
    }

    // 3. send samples to p0
    float *gathered_samples = (float *) malloc(comm_sz * comm_sz * sizeof(float));
    MPI_Gather(samples, comm_sz, MPI_FLOAT, gathered_samples, comm_sz, MPI_FLOAT, 0, MPI_COMM_WORLD);  // recvcount: number of elements for any single receive
    if (my_rank == 0) {
        qsort(gathered_samples, comm_sz * comm_sz);
    }

    // 4. select and broadcast fences
    float *fences = (float *) malloc((comm_sz - 1) * sizeof(float));
    if (my_rank == 0) {
        for (int i = 0; i < comm_sz - 1; i++) {
            fences[i] = gathered_samples[(i + 1) * comm_sz];
        }
    }
    MPI_Bcast(fences, comm_sz - 1, MPI_FLOAT, 0, MPI_COMM_WORLD);

    // 5. extract parts
    int *parts_starts = (int *) malloc(comm_sz * sizeof(int));   // 每个分区的起始数据下标
    int *parts_lengths = (int *) malloc(comm_sz * sizeof(int));  // 每个分区的长度
    int current_index = 0;
    for (int i = 0; i < comm_sz - 1; i++) {
        parts_starts[i] = current_index;
        parts_lengths[i] = 0;
        while ((current_index + 1) < data_length) {
            current_index++;
            if (data[current_index] > fences[i]) {
                break;
            }
            parts_lengths[i]++;
        }
    }
    parts_starts[comm_sz - 1] = current_index;
    parts_lengths[comm_sz - 1] = data_length - current_index;

    // 6. broadcast parts
    int *mparts_starts = (int *) malloc(comm_sz * sizeof(int));   // 每个分区的起始数据下标
    int *mparts_lengths = (int *) malloc(comm_sz * sizeof(int));  // 每个分区的长度
    int mparts_total_length = 0;                                  // 所有分区的数据总长度
    MPI_Alltoall(parts_lengths, 1, MPI_INT, mparts_lengths, 1, MPI_INT, MPI_COMM_WORLD);
    for (int i = 0; i < comm_sz; i++) {
        mparts_starts[i] = mparts_total_length;
        mparts_total_length += mparts_lengths[i];
    }
    float *mparts_data = (float *) malloc(mparts_total_length * sizeof(float));  // 进程对应分区的数据
    MPI_Alltoallv(data, parts_lengths, parts_starts, MPI_FLOAT, mparts_data, mparts_lengths, mparts_starts, MPI_FLOAT, MPI_COMM_WORLD);

    // 7. merge mparts
    float **mparts_heads = (float **) malloc(comm_sz * sizeof(float *));  // 所有分区的首数据地址列表
    for (int i = 0; i < comm_sz; i++) {
        mparts_heads[i] = &mparts_data[mparts_starts[i]];
    }
    float *merged_result = (float *) malloc(mparts_total_length * sizeof(float));  // 所有分区多路归并后的结果
    multiway_merge(mparts_heads, mparts_lengths, comm_sz, merged_result, mparts_total_length);

    // 8. check if in ascending order
    auto end_time = MPI_Wtime();
    if (my_rank == 0) {
        printf("Rank: %d, duration: %.3lfs\n", my_rank, end_time - start_time);
    }
    bool in_order = true;
    for (int i = 1; i < mparts_total_length; i++) {
        if (merged_result[i - 1] > merged_result[i]) {
            in_order = false;
            break;
        }
    }
    if (in_order) {
        printf("Rank: %d, merged result are in ascending order\n", my_rank);
    } else {
        printf("Rank: %d, merged result are not in ascending order !!!\n", my_rank);
    }
    // if (my_rank == 0) {
    //     printf("Rank: %d\n", my_rank);
    //     setbuf(stdout, NULL);
    //     for (int i = 0; i < 2000; i++) {
    //         printf("%.20f\n", merged_result[i * 5000]);
    //     }
    // }

    // *. release memories
    free(merged_result);
    free(mparts_heads);
    free(mparts_lengths);
    free(mparts_starts);
    free(parts_lengths);
    free(parts_starts);
    free(fences);
    free(gathered_samples);
    free(samples);
    free(data);
    MPI_Finalize();
    return 0;
}

void qsort(float *data, uint64_t length) {
    std::qsort(data, length, sizeof(float), [](const void *a, const void *b) -> int {
        float fa = *(float *) a, fb = *(float *) b;
        return (fa > fb) - (fa < fb);
    });
}

struct mpart_item {
    int mpart_index;     // 值对应分区的编号
    int index_in_mpart;  // 值在分区内的下标
    float value;         // 值
};

void multiway_merge(float **heads, int *lengths, int part_count, float *result, int result_length) {
    auto cmp = [](const mpart_item lhs, const mpart_item rhs) -> bool {
        return lhs.value > rhs.value;  // Attention: the priority queue outputs largest elements first
    };
    std::priority_queue<mpart_item, std::vector<mpart_item>, decltype(cmp)> headers(cmp);

    // enqueue multiway headers
    for (int i = 0; i < part_count; i++) {
        headers.push({i, 0, heads[i][0]});
    }

    // multiway merging
    int index = 0;
    while (!headers.empty()) {
        // get minimal value in headers
        auto item = headers.top();
        headers.pop();

        // save to result
        result[index++] = item.value;

        // move to next item
        if (item.index_in_mpart + 1 < lengths[item.mpart_index]) {
            mpart_item next = {item.mpart_index, item.index_in_mpart + 1, heads[item.mpart_index][item.index_in_mpart + 1]};
            headers.push(next);
        }
    }
}
