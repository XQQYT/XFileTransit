@echo off
chcp 65001 >nul
title XFileTransit Build Script

echo ========================================
echo         XFileTransit 构建脚本
echo ========================================

rem 检查参数
if "%1"=="" (
    echo [用法] %0 [动作]
    echo.
    echo 动作:
    echo   release    - 构建Release版本
    echo   debug      - 构建Debug版本
    echo   clean      - 清理构建目录
    echo   help       - 显示帮助信息
    echo.
    pause
    exit /b 1
)

rem 设置变量
set BUILD_TYPE_RELEASE=Release
set BUILD_TYPE_DEBUG=Debug
set TOOLCHAIN_FILE=msvc-toolchain.cmake
set GENERATOR="Visual Studio 17 2022"

rem 根据参数执行不同动作
if /i "%1"=="help" (
    echo [用法] %0 [动作]
    echo.
    echo 动作:
    echo   release    - 构建Release版本
    echo   debug      - 构建Debug版本
    echo   clean      - 清理构建目录
    echo   help       - 显示帮助信息
    echo.
    pause
    exit /b 0
)

if /i "%1"=="clean" (
    echo [信息] 清理构建目录...
    if exist build-Window-%BUILD_TYPE_RELEASE% (
        echo    清理 Release 目录
        rmdir /s /q build-Window-%BUILD_TYPE_RELEASE%
    )
    if exist build-Window-%BUILD_TYPE_DEBUG% (
        echo    清理 Debug 目录
        rmdir /s /q build-Window-%BUILD_TYPE_DEBUG%
    )
    echo [信息] 清理完成
    pause
    exit /b 0
)

if /i "%1"=="release" goto :build_release
if /i "%1"=="debug" goto :build_debug

echo [错误] 未知动作: %1
echo 使用 %0 help 查看帮助
pause
exit /b 1

rem ========== 构建Release ==========
:build_release
echo [信息] 开始构建 Release 版本...

rem 检查工具
call :check_tools
if %errorlevel% neq 0 exit /b 1

rem 清理旧的构建目录
if exist build-Window-%BUILD_TYPE_RELEASE% (
    echo [信息] 清理旧的 Release 目录
    rmdir /s /q build-Window-%BUILD_TYPE_RELEASE%
)

rem 配置
echo [步骤] 配置CMake...
cmake -G %GENERATOR% -DCMAKE_TOOLCHAIN_FILE="%TOOLCHAIN_FILE%" -DCMAKE_BUILD_TYPE=%BUILD_TYPE_RELEASE% -B build-Window-%BUILD_TYPE_RELEASE% .
if %errorlevel% neq 0 (
    echo [错误] CMake配置失败
    pause
    exit /b 1
)

rem 编译
echo [步骤] 编译项目...
cd build-Window-%BUILD_TYPE_RELEASE%
MSbuild XFileTransit.sln /p:Configuration=%BUILD_TYPE_RELEASE% /m
if %errorlevel% neq 0 (
    echo [错误] 编译失败
    cd ..
    pause
    exit /b 1
)
cd ..

rem 检查结果
if exist build-Window-%BUILD_TYPE_RELEASE%\src\Debug\XFileTransit.exe (
    echo [成功] Release版本构建完成: build-Window-%BUILD_TYPE_RELEASE%\src\Debug\XFileTransit.exe
) else (
    echo [警告] 未找到输出文件
)

pause
exit /b 0

rem ========== 构建Debug ==========
:build_debug
echo [信息] 开始构建 Debug 版本...

rem 检查工具
call :check_tools
if %errorlevel% neq 0 exit /b 1

rem 清理旧的构建目录
if exist build-Window-%BUILD_TYPE_DEBUG% (
    echo [信息] 清理旧的 Debug 目录
    rmdir /s /q build-Window-%BUILD_TYPE_DEBUG%
)

rem 配置
echo [步骤] 配置CMake...
cmake -G %GENERATOR% -DCMAKE_TOOLCHAIN_FILE="%TOOLCHAIN_FILE%" -DCMAKE_BUILD_TYPE=%BUILD_TYPE_DEBUG% -B build-Window-%BUILD_TYPE_DEBUG% .
if %errorlevel% neq 0 (
    echo [错误] CMake配置失败
    pause
    exit /b 1
)

rem 编译
echo [步骤] 编译项目...
cd build-Window-%BUILD_TYPE_DEBUG%
MSbuild XFileTransit.sln /p:Configuration=%BUILD_TYPE_DEBUG% /m
if %errorlevel% neq 0 (
    echo [错误] 编译失败
    cd ..
    pause
    exit /b 1
)
cd ..

rem 检查结果
if exist build-Window-%BUILD_TYPE_DEBUG%\src\Debug\XFileTransit.exe (
    echo [成功] Debug版本构建完成: build-Window-%BUILD_TYPE_DEBUG%\src\Debug\XFileTransit.exe
) else (
    echo [警告] 未找到输出文件
)

pause
exit /b 0

rem ========== 检查工具子程序 ==========
:check_tools
where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo [错误] 未找到CMake，请先安装CMake
    exit /b 1
)

where MSbuild >nul 2>nul
if %errorlevel% neq 0 (
    echo [错误] 未找到MSbuild，请安装MSbuild
    exit /b 1
)

echo [信息] 环境检查通过
exit /b 0