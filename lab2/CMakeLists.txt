cmake_minimum_required(VERSION 3.16)
project(lab2)

set(CMAKE_CXX_STANDARD 14)

include_directories($ENV{MSMPI_INC})
include_directories($ENV{MSMPI_INC}/x86)
link_directories($ENV{MSMPI_LIB32})

set(TARGET main.cpp define.h mtx_vec_mul.cpp trap.cpp odd_even_sort.cpp)

add_executable(lab2 ${TARGET})
target_link_libraries(lab2 msmpi.lib msmpifec.lib msmpifmc.lib)
