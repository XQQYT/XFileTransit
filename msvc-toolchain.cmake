# msvc-toolchain.cmake
message(STATUS "Detected MSVC compiler")
        
set(MSVC_TOOLCHAIN "msvc2022" CACHE STRING "Toolchain identifier")
set(QT6_DIR "D:/Qt6.8/6.8.3/msvc2022_64" CACHE PATH "Qt6 MSVC root directory")  #目录下需有bin,include等目录
set(OPENSSL_ROOT_DIR "C:/Program Files/OpenSSL-Win64" CACHE PATH "OpenSSL root directory") #目录下需有include, lib/VC/x64/MT/文件夹
set(BOOST_ROOT "D:/vcpkg/installed/x64-windows/include" CACHE PATH "Boost root directory") #目录下需有boost文件夹，代码中包含头文件格式为#include <boost/beast.hpp>
set(LIBDATACHANNEL_ROOT "D:/vcpkg/installed/x64-windows" CACHE PATH "Libdatachannel root directory") #目录下需有include,lib文件夹
        
set(TOOLCHAIN_LOADED TRUE CACHE INTERNAL "")

