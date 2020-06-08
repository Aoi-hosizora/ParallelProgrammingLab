#include <cstdio>
#include <cstring>
#include <random>

using namespace std;

const int _1K = 1000;
const int _10K = 10000;
const int _100K = 100000;
const int _1M = 1000000;
const int _10M = 10 * _1M;
const int _100M = 10 * _10M;
const int _1G = 10 * _100M;

void generate_random_string_file(const char *char_set, const char *file_name, size_t min_str_len, size_t max_str_len, size_t line_cnt) {
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
    const char char_set_1[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const char char_set_2[] = "abcdefg";
    const char char_set_3[] = "0123456789abcdefgABCDEFG";

    // a-g 1000 2-4
    generate_random_string_file(char_set_2, "1K.txt", 2, 4, _1K);

    // a-g 10000 5-10
    generate_random_string_file(char_set_2, "10K.txt", 5, 10, _10K);

    // 0-9a-gA-G 100000 10-20
    generate_random_string_file(char_set_3, "100K.txt", 10, 20, _100K);

    // 0-9a-gA-G 1000000 10-20
    generate_random_string_file(char_set_3, "1M.txt", 10, 20, _1M);

    // 0-9a-zA-Z 10000000 10-20
    generate_random_string_file(char_set_1, "10M.txt", 10, 20, _10M);

    // 0-9a-zA-Z 100000000 10-50
    generate_random_string_file(char_set_1, "100M.txt", 10, 50, _100M);

    // 0-9a-zA-Z 1000000000 10-50
    generate_random_string_file(char_set_1, "1G.txt", 10, 50, _1G);

    return 0;
}
