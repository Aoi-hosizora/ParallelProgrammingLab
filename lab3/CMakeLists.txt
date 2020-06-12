cmake_minimum_required(VERSION 3.16)
project(lab3)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fopenmp")

include_directories($ENV{MSMPI_INC})
include_directories($ENV{MSMPI_INC}/x86)
link_directories($ENV{MSMPI_LIB32})

add_executable(lab3 main.cpp)
target_link_libraries(lab3 msmpi.lib msmpifec.lib msmpifmc.lib)

# GsLkikfh
# rz -y

# yhi
# mpic++ main.cpp -o main -fopenmp -std=c++0x
# yhrun -N 2 -p free ./main
