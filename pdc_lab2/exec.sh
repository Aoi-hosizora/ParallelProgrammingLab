if [ ! -d build ]; then
    rm -rf build || exit 1
    mkdir -p build
    cd build 
    cmake .. -G "MinGW Makefiles" || exit 1
else
    cd build 
fi

make || exit
cd ..

if [ "$1" = "sort" -a ! -z "$2" ]; then
    mpiexec -n $2 ./build/psrs_sort.exe
fi

# Microsoft MPI (single node)
# mpiexec -n 8 ./build/psrs_sort.exe

# Microsoft MPI (cluster)
# smpd -d -port 8677
# mpiexec -hosts 2 192.168.1.133 4 192.168.1.155 4 ./build/psrs_sort.exe

# MPICH (compile)
# export PATH=/opt/mpich-3.4.3/bin:$PATH
# export LD_LIBRARY_PATH=/opt/mpich-3.4.3/lib:$LD_LIBRARY_PATH
# mpic++ psrs_sort.cpp -I/opt/mpich-3.4.3/include -L/opt/mpich-3.4.3/lib -o psrs_sort.out

# MPICH (single node)
# mpiexec -n ./psrs_sort.out

# MPICH (cluster)
# export MPIR_CVAR_CH3_PORT_RANGE=40000:40100
# mpiexec -n 2 -hosts 192.168.1.133:1,192.168.1.149:1 ./psrs_sort.out
