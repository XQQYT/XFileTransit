#Window环境下的工具链文件
if(WIN32)
    set(CMAKE_SYSTEM_NAME Windows)
    #指定Qt路径
    set(CMAKE_PREFIX_PATH "D:/Qt6.8/6.8.2/mingw_64")

    # 指定编译器
    set(CMAKE_C_COMPILER "D:/Qt6.8/Tools/mingw1310_64/bin/gcc.exe")
    set(CMAKE_CXX_COMPILER "D:/Qt6.8/Tools/mingw1310_64/bin/g++.exe")
    # 设置编译器标志
    set(CMAKE_CXX_FLAGS_INIT "-static-libgcc -static-libstdc++")
endif()