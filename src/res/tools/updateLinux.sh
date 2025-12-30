#!/bin/bash

UPDATE_PACKAGE=$1  # 压缩包的完整路径，如：/home/xqqyt/test/app.tar.gz
INSTALL_DIR=$2     # 安装目录路径

if [ -z "$UPDATE_PACKAGE" ] || [ -z "$INSTALL_DIR" ]; then
    echo "Usage: $0 <update_package_path> <install_directory>"
    echo "Example: $0 /home/xqqyt/test/app.tar.gz /opt/XFileTransit"
    exit 1
fi

echo $INSTALL_DIR

# 检查更新包文件是否存在
if [ ! -f "$UPDATE_PACKAGE" ]; then
    echo "Error: Update package file does not exist - $UPDATE_PACKAGE"
    exit 1
fi

# 检查安装目录是否存在，如果不存在则创建
if [ ! -d "$INSTALL_DIR/XFileTransit" ]; then
    echo "Directory does not exist, creating: $INSTALL_DIR/XFileTransit"
    mkdir -p "$INSTALL_DIR/XFileTransit" || {
        echo "Error: Unable to create directory - $INSTALL_DIR/XFileTransit"
        exit 1
    }
fi

# 切换到目标目录
cd "$INSTALL_DIR/XFileTransit" || {
    echo "Error: Unable to change to directory - $INSTALL_DIR/XFileTransit"
    exit 1
}

echo "Current directory: $(pwd)"
echo "Extracting update package: $UPDATE_PACKAGE"

# 解压前先删除可能已存在的文件
echo "Cleaning existing files..."
if [ -f "XFileTransit" ]; then
    rm -f "XFileTransit"
fi

# 直接解压传递的压缩包文件
tar -xzvf "$UPDATE_PACKAGE" --overwrite

if [ -f "tmp_file/settings.json" ] && [ -f "tmp_file/ConfigMergerLinux" ]; then
    echo "Merging configuration files..."
    chmod +x tmp_file/ConfigMergerLinux 2>/dev/null || true
    tmp_file/ConfigMergerLinux -new "$INSTALL_DIR/XFileTransit/tmp_file/settings.json" -current "$INSTALL_DIR/XFileTransit/settings.json" 2>/dev/null || true
    
    rm -rf tmp_file 2>/dev/null || true
else
    echo "No configuration to merge."
fi
