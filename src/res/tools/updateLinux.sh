#!/bin/bash

UPDATE_PACKAGE=$1  # 压缩包的完整路径，如：/home/xqqyt/test/app.tar.gz
INSTALL_DIR=$2     # 安装目录路径

if [ -z "$UPDATE_PACKAGE" ] || [ -z "$INSTALL_DIR" ]; then
    echo "用法：$0 <更新包路径> <安装目录>"
    echo "示例：$0 /home/xqqyt/test/app.tar.gz /opt/XFileTransit"
    exit 1
fi

# 检查更新包文件是否存在
if [ ! -f "$UPDATE_PACKAGE" ]; then
    echo "错误：更新包文件不存在 - $UPDATE_PACKAGE"
    exit 1
fi

# 检查安装目录是否存在，如果不存在则创建
if [ ! -d "$INSTALL_DIR" ]; then
    echo "目录不存在，正在创建: $INSTALL_DIR"
    mkdir -p "$INSTALL_DIR" || {
        echo "错误：无法创建目录 - $INSTALL_DIR"
        exit 1
    }
fi

# 切换到目标目录
cd "$INSTALL_DIR" || {
    echo "错误：无法切换到目录 - $INSTALL_DIR"
    exit 1
}

echo "当前目录：$(pwd)"
echo "正在解压更新包：$UPDATE_PACKAGE"

# 解压前先删除可能已存在的文件
echo "清理已存在的文件..."
if [ -f "XFileTransit" ]; then
    rm -f "XFileTransit"
fi

# 直接解压传递的压缩包文件
tar -xzvf "$UPDATE_PACKAGE" --overwrite
echo "更新完成！"
date  # 显示时间戳
