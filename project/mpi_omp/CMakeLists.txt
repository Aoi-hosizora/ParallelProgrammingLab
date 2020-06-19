cmake_minimum_required(VERSION 3.16)
project(mpi_omp)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fopenmp")

include_directories($ENV{MSMPI_INC})
include_directories($ENV{MSMPI_INC}/x86)
link_directories($ENV{MSMPI_LIB32})

add_executable(mpi_omp main.cpp)
target_link_libraries(mpi_omp msmpi.lib msmpifec.lib msmpifmc.lib)
