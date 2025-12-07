@echo off
chcp 65001 >nul
title XFileTransit Build Script

echo ========================================
echo         XFileTransit 构建脚本
echo ========================================

rem 检查CMake是否安装
where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo [错误] 未找到CMake，请先安装CMake
    pause
    exit /b 1
)

rem 检查make是否安装
where make >nul 2>nul
if %errorlevel% neq 0 (
    echo [错误] 未找到make，请安装MinGW
    pause
    exit /b 1
)

echo [信息] 环境检查通过
echo.

rem 设置变量
set BUILD_TYPE_RELEASE=Release
set BUILD_TYPE_DEBUG=Debug
set TOOLCHAIN_FILE=mingw-toolchain.cmake
set GENERATOR="MinGW Makefiles"

rem 清理之前的构建目录
if exist build-Window-%BUILD_TYPE_RELEASE% (
    echo [信息] 清理 Release 构建目录...
    rmdir /s /q build-Window-%BUILD_TYPE_RELEASE%
)
if exist build-Window-%BUILD_TYPE_DEBUG% (
    echo [信息] 清理 Debug 构建目录...
    rmdir /s /q build-Window-%BUILD_TYPE_DEBUG%
)

rem 构建Release版本
echo.
echo [信息] 开始构建 Release 版本...
echo.
cmake -G %GENERATOR% -DCMAKE_TOOLCHAIN_FILE="%TOOLCHAIN_FILE%" -DCMAKE_BUILD_TYPE=%BUILD_TYPE_RELEASE% -B build-Window-%BUILD_TYPE_RELEASE% .
if %errorlevel% neq 0 (
    echo [错误] Release版本CMake配置失败
    pause
    exit /b 1
)

cd build-Window-%BUILD_TYPE_RELEASE%
make -j8
if %errorlevel% neq 0 (
    echo [错误] Release版本编译失败
    cd ..
    pause
    exit /b 1
)
cd ..

rem 构建Debug版本
echo.
echo [信息] 开始构建 Debug 版本...
echo.
cmake -G %GENERATOR% -DCMAKE_TOOLCHAIN_FILE="%TOOLCHAIN_FILE%" -DCMAKE_BUILD_TYPE=%BUILD_TYPE_DEBUG% -B build-Window-%BUILD_TYPE_DEBUG% .
if %errorlevel% neq 0 (
    echo [错误] Debug版本CMake配置失败
    pause
    exit /b 1
)

cd build-Window-%BUILD_TYPE_DEBUG%
make -j8
if %errorlevel% neq 0 (
    echo [错误] Debug版本编译失败
    cd ..
    pause
    exit /b 1
)
cd ..

echo.
echo ========================================
echo            构建成功完成！
echo ========================================
echo.

if exist build-Window-%BUILD_TYPE_RELEASE%\XFileTransit.exe (
    echo [成功] Release版本: %cd%\build-Window-%BUILD_TYPE_RELEASE%\XFileTransit.exe
)
echo.
if exist build-Window-%BUILD_TYPE_DEBUG%\XFileTransit.exe (
    echo [成功] Debug版本: %cd%\build-Window-%BUILD_TYPE_DEBUG%\XFileTransit.exe
)

echo.
pause