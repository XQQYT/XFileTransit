cmake -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE="mingw-toolchain.cmake" -DCMAKE_BUILD_TYPE=Release -B build-Release .
cmake -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE="mingw-toolchain.cmake" -DCMAKE_BUILD_TYPE=Debug -B build-Debug .

cd build-Release && make -j8
cd ..
cd build-Debug && make -j8
pause