#!/usr/bin/env bash

Qt6_PATH="/home/xqqyt/Qt6.8/6.8.3/gcc_64"
PROJECT_ROOT=$(pwd)

# 检查Qt路径
if [ ! -d "$Qt6_PATH/lib/cmake/Qt6" ]; then
    echo "错误: Qt6目录不存在: $Qt6_PATH/lib/cmake/Qt6"
    echo "请检查Qt安装路径"
    exit 1
fi

echo "使用Qt路径: $Qt6_PATH"

# 定义构建目录
DEBUG_DIR="build-Linux-Debug"
RELEASE_DIR="build-Linux-Release"

# 函数：构建指定配置
build_config() {
    local BUILD_TYPE=$1
    local BUILD_DIR=$2
    
    echo "========================================"
    echo "构建 $BUILD_TYPE 版本"
    echo "构建目录: $BUILD_DIR"
    echo "========================================"
    
    # 删除并重新创建构建目录
    rm -rf "$BUILD_DIR"
    mkdir -p "$BUILD_DIR"
    
    # 进入构建目录
    cd "$BUILD_DIR" || exit 1
    
    # 配置CMake
    cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
          -DQt6_DIR="$Qt6_PATH/lib/cmake/Qt6" \
          "$PROJECT_ROOT"
    
    if [ $? -eq 0 ]; then
        echo "CMake配置成功，开始编译..."
        make -j$(nproc)
        
        if [ $? -eq 0 ]; then
            echo "$BUILD_TYPE 版本编译成功!"
            echo "可执行文件位于: $PWD/src/XFileTransit"
        else
            echo "$BUILD_TYPE 版本编译失败!"
            exit 1
        fi
    else
        echo "$BUILD_TYPE 版本CMake配置失败!"
        exit 1
    fi
    
    # 返回项目根目录
    cd "$PROJECT_ROOT"
    echo ""
}

# 构建Debug版本
build_config "Debug" "$DEBUG_DIR"

# 构建Release版本
build_config "Release" "$RELEASE_DIR"

echo "========================================"
echo "所有构建完成!"
echo "Debug版本:    $PROJECT_ROOT/$DEBUG_DIR/src/XFileTransit"
echo "Release版本:  $PROJECT_ROOT/$RELEASE_DIR/src/XFileTransit"
echo "========================================"