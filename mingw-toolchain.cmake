#Window环境下的工具链文件
if(WIN32)
    set(CMAKE_SYSTEM_NAME Windows)
    #指定Qt路径
    set(CMAKE_PREFIX_PATH "D:/Qt6.8/6.8.2/mingw_64")

    # 指定编译器
    set(CMAKE_C_COMPILER "D:/Qt6.8/Tools/mingw1310_64/bin/gcc.exe")
    set(CMAKE_CXX_COMPILER "D:/Qt6.8/Tools/mingw1310_64/bin/g++.exe")

    # 指定64位的windres（资源编译器）
    set(CMAKE_RC_COMPILER "D:/Qt6.8/Tools/mingw1310_64/bin/windres.exe")

    # 设置资源编译器标志，指定64位架构
    set(CMAKE_RC_FLAGS "-F pe-x86-64")
    # 设置编译器标志
    set(CMAKE_CXX_FLAGS_INIT "-static-libgcc -static-libstdc++")
endif()