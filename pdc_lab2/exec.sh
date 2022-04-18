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
