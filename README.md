# ParallelProgrammingLab

+ 2020 SCUT Parallel Programming Experiment and Final Project (with bad-smell and unoptimized code)
+ 2022 SCUT Parallel and Distributed Computing Experiment

### Contents

+ lab1 - OpenMP
    + matrix-vector multiplication
    + trapezoidal integration
    + odd-even sort
+ lab2 - MPI
    + matrix-vector multiplication
    + trapezoidal integration
    + odd-even sort
+ lab3 - OpenMP + MPI
    + matrix-vector multiplication
+ lab4 - OpenMP + MPI
    + trapezoidal integration
+ groupby - Sequential, OpenMP, MPI, OpenMP + MPI
    + group by
+ pdc - MPI (2022)
    + PSRS sort

### Build and run

1. Use CLion

[clion_configuration.png](./lab4/clion_configuration.png)

2. Compile manually

```bash
# build
cd lab2
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
make -j8

# run
lab2.exe # single process
mpiexec -n 8 lab2.exe # multiple processes
```
