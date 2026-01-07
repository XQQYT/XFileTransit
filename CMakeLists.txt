cmake_minimum_required(VERSION 3.16)
project(XFileTransit LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    macro(check_toolchain_var var_name default_value)
        if(NOT DEFINED ${var_name})
            set(${var_name} ${default_value} CACHE STRING "Toolchain variable")
            message(STATUS "Setting default for ${var_name}: ${default_value}")
        endif()
    endmacro()

    check_toolchain_var(MSVC_TOOLCHAIN "default")
    message(STATUS "Using toolchain: ${MSVC_TOOLCHAIN}")

    if(NOT DEFINED TOOLCHAIN_LOADED)
        set(TOOLCHAIN_LOADED FALSE CACHE INTERNAL "")
        message(WARNING "Toolchain not properly loaded! Some features may not work.")
    endif()

    if(DEFINED QT6_DIR AND EXISTS ${QT6_DIR})
        set(CMAKE_PREFIX_PATH ${QT6_DIR} ${CMAKE_PREFIX_PATH})
        message(STATUS "Using Qt6 from: ${QT6_DIR}")
    endif()

    if(DEFINED OPENSSL_ROOT_DIR AND EXISTS ${OPENSSL_ROOT_DIR})
        message(STATUS "Using OpenSSL from: ${OPENSSL_ROOT_DIR}")
    endif()

    if(DEFINED BOOST_ROOT AND EXISTS ${BOOST_ROOT})
        message(STATUS "Using BOOST_ROOT from: ${BOOST_ROOT}")
    endif()

    if(DEFINED LIBDATACHANNEL_ROOT AND EXISTS ${LIBDATACHANNEL_ROOT})
        message(STATUS "Using LIBDATACHANNEL_ROOT from: ${LIBDATACHANNEL_ROOT}")
    endif()
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Quick Widgets Gui Network QuickControls2 LinguistTools)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_subdirectory(src)