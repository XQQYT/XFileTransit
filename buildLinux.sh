#!/usr/bin/env bash

Qt6_PATH="/home/qyt/Qt6.8/6.8.3/gcc_64"
BUILD_DIR="build-Linux"
PROJECT_ROOT=$(pwd)

if [ ! -d "$Qt6_PATH/lib/cmake/Qt6" ]; then
    echo "错误: Qt6目录不存在: $Qt6_PATH/lib/cmake/Qt6"
    echo "请检查Qt安装路径"
    exit 1
fi

echo "使用Qt路径: $Qt6_PATH"
echo "构建目录: $BUILD_DIR"

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR" || exit 1

cmake -DCMAKE_BUILD_TYPE=Debug \
      -DQt6_DIR="$Qt6_PATH/lib/cmake/Qt6" \
      "$PROJECT_ROOT"

if [ $? -eq 0 ]; then
    echo "CMake配置成功，开始编译..."
    make -j$(nproc)
    
    if [ $? -eq 0 ]; then
        echo "编译成功!"
        echo "可执行文件位于: $PWD/src/XFileTransit"
    else
        echo "编译失败!"
        exit 1
    fi
else
    echo "CMake配置失败!"
    exit 1
fi