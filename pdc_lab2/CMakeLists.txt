cmake_minimum_required(VERSION 3.16)
project(psrs_sort)

set(CMAKE_CXX_STANDARD 14)

include_directories($ENV{MSMPI_INC})
include_directories($ENV{MSMPI_INC}/x64)
link_directories($ENV{MSMPI_LIB64})

include_directories(".")

add_executable(data_generator data_generator.cpp)

add_executable(data_partition data_partition.cpp)

add_executable(psrs_sort psrs_sort.cpp)
target_link_libraries(psrs_sort msmpi.lib msmpifec.lib msmpifmc.lib)
