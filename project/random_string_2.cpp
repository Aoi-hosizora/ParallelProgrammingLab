/**
 * 并行程序大作业的测试数据：
 * 由大写字母 A - H 随机组成，一个串一行，回车换行。
 * 1）第一个数据集：10M条数据，串的长度5-10；
 * 2）第二个数据集：100M条数据，串的长度5-10；
 * 3）第三个数据集：1G条数据，串的长度5-10.
 */
#include <cstdio>
#include <cstring>
#include <random>

using namespace std;

const int _1M = 1000000;
const int _10M = 10 * _1M;
const int _100M = 10 * _10M;
const int _1G = 10 * _100M;

void generate_random_string_file(const char *file_name, size_t min_str_len, size_t max_str_len, size_t line_cnt) {
    const char char_set[] = "ABCDEFGH";
    default_random_engine e(0L);

    uniform_int_distribution<size_t> random_index(0, strlen(char_set) - 1);
    uniform_int_distribution<size_t> random_length(min_str_len, max_str_len);

    char buf[max_str_len + 1];

    FILE *f_10m = fopen(file_name, "w+");
    for (size_t i = 0; i < line_cnt; ++i) {
        size_t len = random_length(e);
        for (size_t j = 0; j < len; ++j) {
            buf[j] = char_set[random_index(e)];
        }
        buf[len] = 0;
        fprintf(f_10m, "%s\n", buf);
    }
    fclose(f_10m);
}

int main() {
    // 第一个数据集：10M条数据，串的长度5-10
    generate_random_string_file("10M.txt", 5, 10, _10M);

    // 第二个数据集：100M条数据，串的长度5-10；
    generate_random_string_file("100M.txt", 5, 10, _100M);

    // 第三个数据集：1G条数据，串的长度5-10.
    generate_random_string_file("1G.txt", 5, 10, _1G);

    return 0;
}
