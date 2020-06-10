cmake_minimum_required(VERSION 3.16)
project(mpi)

set(CMAKE_CXX_STANDARD 14)

include_directories($ENV{MSMPI_INC})
include_directories($ENV{MSMPI_INC}/x86)
link_directories($ENV{MSMPI_LIB32})

add_executable(mpi main.cpp)
target_link_libraries(mpi msmpi.lib msmpifec.lib msmpifmc.lib)
