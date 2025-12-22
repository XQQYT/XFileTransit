#!/bin/bash

# 用法：
# pwd: XFileTransit
# bash ./tools/package/generate_release_deb.sh ./build-Linux-Debug/src/XFileTransit 1.1.0 /home/xqqyt/Qt6.8/6.8.3/gcc_64

EXECUTABLE="$1"
VERSION="$2"
QT_GCC_PATH="$3"

PACKAGE_DIR="/tmp/XFileTransit-Deb"

if [[ -z "$EXECUTABLE" || -z "$VERSION" || -z "$QT_GCC_PATH" ]]; then
    echo "用法: $0 <可执行文件路径> <版本号> <Qt_gcc路径>"
    echo "示例: $0 ./build-Linux-Debug/src/XFileTransit 1.1.0 /home/xqqyt/Qt6.8/6.8.3/gcc_64"
    exit 1
fi

# 检查必要的工具
for cmd in dpkg-deb patchelf ldd; do
    if ! command -v $cmd &> /dev/null; then
        echo "错误: 需要安装 $cmd"
        echo "请运行: sudo apt install dpkg-dev patchelf"
        exit 1
    fi
done

# 设置包信息
PACKAGE_NAME="XFileTransit_${VERSION}_amd64.deb"
DEB_ROOT="$PACKAGE_DIR/XFileTransit_${VERSION}_deb"

echo "======================================"
echo "开始打包 XFileTransit $VERSION"
echo "可执行文件: $EXECUTABLE"
echo "Qt 路径: $QT_GCC_PATH"
echo "输出目录: $DEB_ROOT"
echo "======================================"

# 清理旧的构建目录
rm -rf "$DEB_ROOT"
mkdir -p "$DEB_ROOT"

# 创建 DEB 包的标准目录结构
mkdir -p "$DEB_ROOT/DEBIAN"
mkdir -p "$DEB_ROOT/opt/XFileTransit"
mkdir -p "$DEB_ROOT/opt/XFileTransit/lib"
mkdir -p "$DEB_ROOT/opt/XFileTransit/plugins"
mkdir -p "$DEB_ROOT/usr/share/applications"
mkdir -p "$DEB_ROOT/usr/share/icons/hicolor/256x256/apps"

# control
cat > "$DEB_ROOT/DEBIAN/control" << EOF
Package: xfiletransit
Version: $VERSION
Architecture: amd64
Maintainer: XQQYT<xqqyt0502@163.com>
Depends: libc6 (>= 2.34), libstdc++6 (>= 11), libgcc-s1 (>= 4.2), libgl1, libxcb1, libx11-6
Recommends: libxcb-xinerama0, libxcb-icccm4, libxcb-image0, libxcb-keysyms1, libxcb-randr0, libxcb-render-util0, libxcb-xkb1, libxkbcommon-x11-0
Section: utils
Priority: optional
Homepage: https://github.com/XQQYT/XFileTransit
Description: Fast and secure file transfer application with Qt QML UI
 XFileTransit is a modern file transfer application built with Qt6 and QML.
EOF

# postinst
cat > "$DEB_ROOT/DEBIAN/postinst" << 'EOF'
#!/bin/bash
set -e

if [ -x "$(command -v update-desktop-database)" ]; then
    update-desktop-database /usr/share/applications 2>/dev/null || true
fi

if [ -x "$(command -v gtk-update-icon-cache)" ]; then
    gtk-update-icon-cache -f /usr/share/icons/hicolor 2>/dev/null || true
fi

chmod 755 /opt/XFileTransit/XFileTransit 2>/dev/null || true

echo "XFileTransit $1 安装完成！"
echo "您可以在应用程序菜单中找到它，或直接运行: /opt/XFileTransit/XFileTransit"
EOF
chmod 755 "$DEB_ROOT/DEBIAN/postinst"

# prerm
cat > "$DEB_ROOT/DEBIAN/prerm" << 'EOF'
#!/bin/bash
set -e

echo "正在卸载 XFileTransit..."
EOF
chmod 755 "$DEB_ROOT/DEBIAN/prerm"

echo "复制可执行文件..."
if [[ -f "$EXECUTABLE" ]]; then
    cp "$EXECUTABLE" "$DEB_ROOT/opt/XFileTransit/XFileTransit"
    chmod +x "$DEB_ROOT/opt/XFileTransit/XFileTransit"
else
    echo "错误: 找不到可执行文件: $EXECUTABLE"
    exit 1
fi

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
        cp "$icon_src" "$DEB_ROOT/usr/share/icons/hicolor/256x256/apps/xfiletransit.png"
        cp "$icon_src" "$DEB_ROOT/opt/XFileTransit/icon.png"
        ICON_FOUND=true
        echo "使用图标: $icon_src"
        break
    fi
done

if [[ "$ICON_FOUND" == false ]]; then
    echo "警告: 未找到应用程序图标，使用默认占位符"
    convert -size 256x256 xc:#4a90e2 -fill white -pointsize 72 -gravity center -draw "text 0,0 'XFT'" "$DEB_ROOT/usr/share/icons/hicolor/256x256/apps/xfiletransit.png" 2>/dev/null || \
    echo "注意: 未安装 imagemagick,跳过图标创建"
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

for lib in "${QT_LIBS[@]}"; do
    lib_path="$QT_GCC_PATH/lib/$lib"
    if [[ -f "$lib_path" ]]; then
        echo "复制 Qt 库: $lib"
        cp -f "$lib_path"* "$DEB_ROOT/opt/XFileTransit/lib/" 2>/dev/null || true
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
            cp -f "$lib" "$DEB_ROOT/opt/XFileTransit/lib/" 2>/dev/null || true
        fi
    fi
done

# 复制 Qt 插件
echo "复制 Qt 插件..."
PLUGIN_DIRS=("platforms" "xcbglintegrations" "imageformats" "platformthemes" "tls")

for plugin_dir in "${PLUGIN_DIRS[@]}"; do
    src_dir="$QT_GCC_PATH/plugins/$plugin_dir"
    dest_dir="$DEB_ROOT/opt/XFileTransit/plugins/$plugin_dir"
    
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
    dest_dir="$DEB_ROOT/opt/XFileTransit/qml/$qml_module"
    
    if [[ -d "$src_dir" ]]; then
        echo "复制 QML 模块: $qml_module"
        mkdir -p "$dest_dir"
        cp -r "$src_dir/"* "$dest_dir/" 2>/dev/null || true
    fi
done

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
MAIN_EXECUTABLE="$DEB_ROOT/opt/XFileTransit/XFileTransit"
if [[ -f "$MAIN_EXECUTABLE" ]]; then
    set_rpath "$MAIN_EXECUTABLE" '$ORIGIN/lib'
fi

# 设置 Qt 插件的 RPATH
echo "设置 Qt 插件的 RPATH..."
find "$DEB_ROOT/opt/XFileTransit/plugins" -name "*.so" -type f | while read -r plugin; do
    set_rpath "$plugin" '$ORIGIN/../../lib'
done

# 设置 Qt 库的 RPATH
echo "设置 Qt 库的 RPATH..."
find "$DEB_ROOT/opt/XFileTransit/lib" -name "libQt6*.so*" -type f | while read -r qt_lib; do
    set_rpath "$qt_lib" '$ORIGIN'
done


# 创建桌面文件
cat > "$DEB_ROOT/usr/share/applications/xfiletransit.desktop" << EOF
[Desktop Entry]
Type=Application
Name=XFileTransit
GenericName=File Transfer Tool
Comment=Fast and secure file transfer application
Exec=/opt/XFileTransit/XFileTransit
Icon=xfiletransit
Terminal=false
Categories=Network;FileTransfer;Utility;
Keywords=file;transfer;network;qt;qml;
StartupNotify=true
MimeType=
X-AppImage-Version=$VERSION
EOF

# 创建符号链接到 /usr/local/bin
mkdir -p "$DEB_ROOT/usr/local/bin"
ln -sf "/opt/XFileTransit/XFileTransit" "$DEB_ROOT/usr/local/bin/xfiletransit"

# 计算包大小
echo "计算包大小..."
INSTALLED_SIZE=$(du -sk "$DEB_ROOT/opt/XFileTransit" | awk '{print $1}')
sed -i "s/^Installed-Size:.*/Installed-Size: $INSTALLED_SIZE/" "$DEB_ROOT/DEBIAN/control"

# 打包 DEB
echo "正在创建 DEB 包..."
cd "$PACKAGE_DIR"
dpkg-deb --build "$(basename "$DEB_ROOT")" "$PACKAGE_NAME"

# 验证 DEB 包
if [[ -f "$PACKAGE_NAME" ]]; then
    echo "======================================"
    echo "DEB 包创建成功!"
    echo "文件: $PACKAGE_DIR/$PACKAGE_NAME"
    echo "大小: $(du -h "$PACKAGE_NAME" | awk '{print $1}')"
    echo "======================================"
    
    # 显示包信息
    echo "包信息:"
    dpkg-deb -I "$PACKAGE_NAME" | head -20
else
    echo "错误: 创建 DEB 包失败!"
    exit 1
fi