cd build || exit
make || exit
cd ..
if [ "$1" = "sort" -a ! -z "$2" ]; then
    mpiexec -n $2 ./build/psrs_sort.exe
fi

# ./exec.sh sort 8
