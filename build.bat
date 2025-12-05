@REM cmake -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE="mingw-toolchain.cmake" -DCMAKE_BUILD_TYPE=Release -B build .
cmake -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE="mingw-toolchain.cmake" -DCMAKE_BUILD_TYPE=Debug -B build .

cd build && make -j8
pause