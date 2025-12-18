#!/bin/bash

# 用法：
# pwd: XFileTransit
# bash ./tools/package/generate_release_deb.sh ./build-Linux-Debug/src/XFileTransit 1.1.0 /home/xqqyt/Qt6.8/6.8.3/gcc_64

EXECUTABLE="$1"
VERSION="$2"
QT_GCC_PATH="$3"

PACKAGE_DIR="/tmp/XFileTransit-Deb"

if [[ -z "$EXECUTABLE" || -z "$PACKAGE_DIR" || -z "$VERSION" || -z "$QT_GCC_PATH" ]]; then
    echo "Usage: $0 <executable_path> <temp_output_path> <Version> <qt_gcc_path>"
    exit 1
fi

# 设置包的名称
PACKAGE_NAME="XFileTransit_${VERSION}_amd64.deb"
DEB_TEMPLATE="tools/package/DebTemplete"
DEB_DIR="$PACKAGE_DIR/XFileTransit_${VERSION}_tmp"
DEB_TEMPLATE_DIR="$DEB_DIR/DebTemplete"

echo "deb template dir is $DEB_TEMPLATE_DIR"

# 创建目标包目录
rm -rf "$DEB_DIR"  # 清除已存在的目标目录
mkdir -p "$DEB_DIR"

# 复制Deb模板
cp -r "$DEB_TEMPLATE" "$DEB_DIR"

rm -rf "$DEB_TEMPLATE_DIR/.git"
rm -f "$DEB_TEMPLATE_DIR/.gitignore"

# 更新版本号到 control 文件
sed -i "s/Version: .*/Version: $VERSION/" "$DEB_TEMPLATE_DIR/DEBIAN/control"

# 拷贝依赖的 Qt 库到 lib 文件夹
LIB_DIR="$DEB_TEMPLATE_DIR/opt/XFileTransit/lib"
mkdir -p "$LIB_DIR"

echo "Collecting shared library dependencies for $EXECUTABLE..."

# 提取依赖库
ldd "$EXECUTABLE" | awk '{ print $3 }' | grep "^/" | while read -r lib; do
    if [[ -f "$lib" ]]; then
        echo "Copying: $lib"
        cp -u "$lib" "$LIB_DIR"
    fi
done

cp "$QT_GCC_PATH/lib/libQt6QuickControls2.so.6.8.3" "$LIB_DIR/"
# cp "$QT_GCC_PATH/lib/libQt5DBus.so.5" "$LIB_DIR/"


function patch_rpath {
    local target_file="$1"
    local rpath_value="$2"
    if [[ -f "$target_file" ]]; then
        echo "Patching RPATH of $target_file -> $rpath_value"
        patchelf --set-rpath "$rpath_value" "$target_file"
    else
        echo "Not found: $target_file"
    fi
}

# 拷贝 Qt 插件目录
for plugin_dir in platforms xcbglintegrations; do
    SRC="$QT_GCC_PATH/plugins/$plugin_dir"
    DEST="$DEB_TEMPLATE_DIR/opt/XFileTransit/$plugin_dir"
    if [[ -d "$SRC" ]]; then
        echo "Copying $plugin_dir -> $DEST"
        mkdir -p "$DEST"
        cp -ru "$SRC/"* "$DEST/"
    else
        echo "Plugin directory not found: $SRC"
    fi
done

patch_rpath "$DEB_TEMPLATE_DIR/opt/XFileTransit/platforms/libqxcb.so" '$ORIGIN/../lib'

# 拷贝构建好的可执行文件
cp $EXECUTABLE "$DEB_TEMPLATE_DIR/opt/XFileTransit/XFileTransit"

cp "src/res/logo/logo_small.png" "$DEB_TEMPLATE_DIR/opt/XFileTransit/logo.png"

# 打包为deb文件
dpkg-deb --build "$DEB_TEMPLATE_DIR" "$PACKAGE_DIR/$PACKAGE_NAME"

echo "Done. DEB package created: $PACKAGE_DIR/$PACKAGE_NAME"
