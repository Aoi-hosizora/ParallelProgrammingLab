#include <cstdint>

#ifndef _PREDEFINED_H_
#define _PREDEFINED_H_

// 数据量
#define DATA 256
// #define DATA 1024
// #define DATA 4096

// 进程数
const int P = 2;
// const int P = 4;
// const int P = 8;

#if DATA == 256
const uint64_t N = (uint64_t) 256 * 1024 * 1024;
const char *FILENAME = "./256M.txt";
const char *FILENAME_FMT = "./256M_%d.txt";
#elif DATA == 1024
const uint64_t N = (uint64_t) 1024 * 1024 * 1024;
const char *FILENAME = "./1G.txt";
const char *FILENAME_FMT = "./1G_%d.txt";
#elif DATA == 4096
const uint64_t N = (uint64_t) 4 * 1024 * 1024 * 1024;
const char *FILENAME = "./4G.txt";
const char *FILENAME_FMT = "./4G_%d.txt";
#endif

const int MAX_PATH_LEN = 128;

#endif  // _PREDEFINED_H_
