# msvc-toolchain.cmake
message(STATUS "Detected MSVC compiler")
        
set(MSVC_TOOLCHAIN "msvc2022" CACHE STRING "Toolchain identifier")
set(QT6_DIR "D:/Qt6.8/6.8.3/msvc2022_64" CACHE PATH "Qt6 MSVC root directory")
set(OPENSSL_ROOT_DIR "C:/Program Files/OpenSSL-Win64" CACHE PATH "OpenSSL root directory")
set(BOOST_ROOT "D:/vcpkg/installed/x64-windows" CACHE PATH "Boost root directory")
set(LIBDATACHANNEL_ROOT "D:/vcpkg/installed/x64-windows" CACHE PATH "Libdatachannel root directory")
        
set(TOOLCHAIN_LOADED TRUE CACHE INTERNAL "")

