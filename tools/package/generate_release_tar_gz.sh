#!/bin/bash

# ============================================
# XFileTransit tar.gz 打包脚本
# ============================================

# 用法：
# pwd: XFileTransit
# bash ./tools/package/generate_release_tar_gz.sh ./build-Linux-Debug/src/XFileTransit 1.1.0 /home/xqqyt/Qt6.8/6.8.3/gcc_64

EXECUTABLE="$1"
VERSION="$2"
QT_GCC_PATH="$3"

PACKAGE_DIR="/tmp/XFileTransit-tar-gz"

if [[ -z "$EXECUTABLE" || -z "$VERSION" || -z "$QT_GCC_PATH" ]]; then
    echo "用法: $0 <可执行文件路径> <版本号> <Qt_gcc路径>"
    echo "示例: $0 ./build-Linux-Debug/src/XFileTransit 1.1.0 /home/xqqyt/Qt6.8/6.8.3/gcc_64"
    exit 1
fi

# 检查必要的工具
for cmd in patchelf ldd; do
    if ! command -v $cmd &> /dev/null; then
        echo "错误: 需要安装 $cmd"
        echo "请运行: sudo apt install patchelf"
        exit 1
    fi
done

# 设置包信息
PACKAGE_NAME="XFileTransit-${VERSION}_amd64.tar.gz"
TAR_ROOT="$PACKAGE_DIR/XFileTransit_${VERSION}_amd64"

echo "======================================"
echo "开始打包 XFileTransit $VERSION"
echo "可执行文件: $EXECUTABLE"
echo "Qt 路径: $QT_GCC_PATH"
echo "输出目录: $TAR_ROOT"
echo "======================================"

# 清理旧的构建目录
rm -rf "$TAR_ROOT"
mkdir -p "$TAR_ROOT"


echo "复制应用程序图标..."
ICON_SOURCES=(
    "src/res/logo/logo_small.png"
    "src/res/logo/logo.png"
    "logo.png"
    "icon.png"
)

ICON_FOUND=false
for icon_src in "${ICON_SOURCES[@]}"; do
    if [[ -f "$icon_src" ]]; then
        cp "$icon_src" "$TAR_ROOT/icon.png"
        ICON_FOUND=true
        echo "使用图标: $icon_src"
        break
    fi
done

if [[ "$ICON_FOUND" == false ]]; then
    echo "警告: 未找到应用程序图标，使用默认占位符"
    convert -size 256x256 xc:#4a90e2 -fill white -pointsize 72 -gravity center -draw "text 0,0 'XFT'" "$TAR_ROOT/icon.png" 2>/dev/null || \
    echo "注意: 未安装 imagemagick,跳过图标创建"
fi

echo "复制可执行文件..."
if [[ -f "$EXECUTABLE" ]]; then
    cp "$EXECUTABLE" "$TAR_ROOT/XFileTransit"
    chmod +x "$TAR_ROOT/XFileTransit"
else
    echo "错误: 找不到可执行文件: $EXECUTABLE"
    exit 1
fi

# 收集 Qt6 核心库
echo "收集 Qt6 库依赖..."
QT_LIBS=(
    "libQt6Core.so.6"
    "libQt6Gui.so.6"
    "libQt6Qml.so.6"
    "libQt6Quick.so.6"
    "libQt6QuickControls2.so.6"
    "libQt6QuickTemplates2.so.6"
    "libQt6Widgets.so.6"
    "libQt6Network.so.6"
    "libQt6DBus.so.6"
    "libQt6XcbQpa.so.6"
    "libQt6OpenGL.so.6"
    "libQt6QmlMeta.so.6"
    "libQt6QmlWorkerScript.so.6"
    "libQt6QmlModels.so.6"
    "libQt6QuickControls2Impl.so.6"
    "libQt6QuickControls2Basic.so.6"
    "libQt6QuickControls2BasicStyleImpl.so.6"
    "libQt6QuickLayouts.so.6"
    "libQt6LabsPlatform.so.6"
)

mkdir "$TAR_ROOT/lib"

for lib in "${QT_LIBS[@]}"; do
    lib_path="$QT_GCC_PATH/lib/$lib"
    if [[ -f "$lib_path" ]]; then
        echo "复制 Qt 库: $lib"
        cp -f "$lib_path"* "$TAR_ROOT/lib/" 2>/dev/null || true
    else
        echo "警告: 未找到 Qt 库: $lib_path"
    fi
done

# 复制其他系统依赖库
echo "收集系统依赖库..."
ldd "$EXECUTABLE" | awk '{ print $3 }' | grep "^/" | while read -r lib; do
    if [[ -f "$lib" ]]; then
        lib_name=$(basename "$lib")
        # 跳过 Qt 库
        if [[ "$lib_name" != libQt6* ]]; then
            echo "复制系统库: $lib_name"
            cp -f "$lib" "$TAR_ROOT/lib/" 2>/dev/null || true
        fi
    fi
done

EXCLUDE_LIB=(
    "libc.so.6"
    "libdl.so.2"
    "libm.so.6"
    "libpthread.so.0"
    "librt.so.1"
    "libresolv.so.2"
    "libgcc_s.so.1"
    "libstdc++.so.6"
    "libz.so.1"
    "libX11.so.6"
    "libXau.so.6"
    "libXdmcp.so.6"
    "libxcb.so.1"
    "libxkbcommon.so.0"
    "libEGL.so.1"
    "libGL.so.1"
    "libGLX.so.0"
    "libOpenGL.so.0"
    "libbz2.so.1.0"
    "libexpat.so.1"
    "liblzma.so.5"
    "libzstd.so.1"
)

for lib in "${EXCLUDE_LIB[@]}"; do
    echo "移除 $lib"
    rm -f "$TAR_ROOT/lib/$lib" 2>/dev/null || true
done

# 复制 Qt 插件
echo "复制 Qt 插件..."
PLUGIN_DIRS=("platforms" "xcbglintegrations" "imageformats" "platformthemes" "tls")

for plugin_dir in "${PLUGIN_DIRS[@]}"; do
    src_dir="$QT_GCC_PATH/plugins/$plugin_dir"
    dest_dir="$TAR_ROOT/plugins/$plugin_dir"
    
    if [[ -d "$src_dir" ]]; then
        echo "复制插件目录: $plugin_dir"
        mkdir -p "$dest_dir"
        if [[ "$plugin_dir" == "platforms" ]]; then
            # cp "$src_dir/libqxcb.so" "$dest_dir/" 2>/dev/null || true
            cp -r "$src_dir/"*.so "$dest_dir/" 2>/dev/null || true
        elif [[ "$plugin_dir" == "xcbglintegrations" ]]; then
            cp "$src_dir/libqxcb-glx-integration.so" "$dest_dir/" 2>/dev/null || true
        else
            cp -r "$src_dir/"* "$dest_dir/" 2>/dev/null || true
        fi
    fi
done

# 复制 QML 模块
echo "复制 QML 模块..."
QML_MODULES=("QtQuick" "QtQuick/Controls" "QtQuick/Templates" "QtQuick/Window" "QtQml" "Qt/labs/platform")

for qml_module in "${QML_MODULES[@]}"; do
    src_dir="$QT_GCC_PATH/qml/$qml_module"
    dest_dir="$TAR_ROOT/qml/$qml_module"
    
    if [[ -d "$src_dir" ]]; then
        echo "复制 QML 模块: $qml_module"
        mkdir -p "$dest_dir"
        cp -r "$src_dir/"* "$dest_dir/" 2>/dev/null || true
    fi
done

mkdir "$TAR_ROOT/tmp_file"
GOOS=linux GOARCH=amd64 go build -ldflags="-s -w" -o "$TAR_ROOT"/tmp_file/ConfigMergerLinux ./tools/update/configMerger.go
cp "src/res/settings/settings.json" "$TAR_ROOT/tmp_file/settings.json"

# 设置 RPATH 的函数
function set_rpath() {
    local target_file="$1"
    local rpath_value="$2"
    
    if [[ -f "$target_file" ]]; then
        echo "设置 RPATH: $(basename "$target_file") -> $rpath_value"
        patchelf --set-rpath "$rpath_value" "$target_file" 2>/dev/null || \
        echo "警告: 无法设置 $target_file 的 RPATH"
    fi
}

# 设置主可执行文件的 RPATH
echo "设置可执行文件的 RPATH..."
MAIN_EXECUTABLE="$TAR_ROOT/XFileTransit"
if [[ -f "$MAIN_EXECUTABLE" ]]; then
    set_rpath "$MAIN_EXECUTABLE" '$ORIGIN/lib'
fi

# 设置 Qt 插件的 RPATH
echo "设置 Qt 插件的 RPATH..."
find "$TAR_ROOT/plugins" -name "*.so" -type f | while read -r plugin; do
    set_rpath "$plugin" '$ORIGIN/../../lib'
done

# 设置 Qt 库的 RPATH
echo "设置 Qt 库的 RPATH..."
find "$TAR_ROOT/lib" -name "libQt6*.so*" -type f | while read -r qt_lib; do
    set_rpath "$qt_lib" '$ORIGIN'
done

echo "正在创建压缩包..."
cd "$TAR_ROOT"
tar -czf "../$PACKAGE_NAME" .

echo "tar.gz 压缩包创建成功!"
echo "文件: $PACKAGE_DIR/$PACKAGE_NAME"
echo "大小: $(du -h "$PACKAGE_DIR/$PACKAGE_NAME" | awk '{print $1}')"