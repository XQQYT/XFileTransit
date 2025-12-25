#!/bin/bash

# ============================================
# XFileTransit AppImage 打包脚本
# 基于 tar.gz 打包脚本修改
# ============================================

# 用法：
# bash ./generate_release_appimage.sh <可执行文件路径> <版本号> <Qt_gcc路径>
# 示例: bash ./tools/package/generate_release_appimage.sh ./build-Linux-Debug/src/XFileTransit 1.1.0 /home/xqqyt/Qt6.8/6.8.3/gcc_64

set -e

EXECUTABLE="$1"
VERSION="$2"
QT_GCC_PATH="$3"

if [[ -z "$EXECUTABLE" || -z "$VERSION" || -z "$QT_GCC_PATH" ]]; then
    echo "用法: $0 <可执行文件路径> <版本号> <Qt_gcc路径>"
    echo "示例: $0 ./build-Linux-Debug/src/XFileTransit 1.1.0 /home/xqqyt/Qt6.8/6.8.3/gcc_64"
    exit 1
fi

# 检查必要的工具
for cmd in patchelf ldd wget; do
    if ! command -v $cmd &> /dev/null; then
        echo "错误: 需要安装 $cmd"
        echo "请运行: sudo apt install patchelf wget"
        exit 1
    fi
done

# 设置包信息
APP_NAME="XFileTransit"
ARCH="x86_64"
PACKAGE_NAME="${APP_NAME}_${VERSION}_${ARCH}.AppImage"

# 工作目录结构
WORK_DIR="/tmp/XFileTransit-AppImage"
APP_DIR="${WORK_DIR}/${APP_NAME}.AppDir"
TAR_ROOT="${APP_DIR}/usr"

echo "======================================"
echo "开始打包 XFileTransit AppImage $VERSION"
echo "可执行文件: $EXECUTABLE"
echo "Qt 路径: $QT_GCC_PATH"
echo "输出文件: $PACKAGE_NAME"
echo "======================================"

# 清理旧的构建目录
rm -rf "$WORK_DIR"
mkdir -p "$TAR_ROOT" "$APP_DIR"

echo "1. 复制应用程序文件..."

# 查找并复制图标
ICON_SOURCES=(
    "src/res/logo/logo_small.png"
    "src/res/logo/logo.png"
    "logo.png"
    "icon.png"
)

ICON_FOUND=false
ICON_PATH=""
for icon_src in "${ICON_SOURCES[@]}"; do
    if [[ -f "$icon_src" ]]; then
        ICON_PATH="$icon_src"
        ICON_FOUND=true
        echo "使用图标: $icon_src"
        break
    fi
done

# 复制可执行文件
if [[ -f "$EXECUTABLE" ]]; then
    cp "$EXECUTABLE" "$TAR_ROOT/XFileTransit"
    chmod +x "$TAR_ROOT/XFileTransit"
else
    echo "错误: 找不到可执行文件: $EXECUTABLE"
    exit 1
fi

echo "复制配置文件..."
if [[ -d "src/res/settings" ]]; then
    # 只复制settings目录中的文件，不复制目录本身
    cp "src/res/settings"/* "$TAR_ROOT/" 2>/dev/null || true
    echo "已复制 settings 目录中的文件到: $TAR_ROOT/"
else
    echo "警告: 未找到 src/res/settings 目录"
fi

echo "2. 收集库依赖..."

# 创建库目录
mkdir -p "$TAR_ROOT/lib"

# 收集 Qt6 核心库
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

echo "3. 复制 Qt 插件和 QML 模块..."

# 复制 Qt 插件
PLUGIN_DIRS=("platforms" "xcbglintegrations" "imageformats" "platformthemes" "tls")

for plugin_dir in "${PLUGIN_DIRS[@]}"; do
    src_dir="$QT_GCC_PATH/plugins/$plugin_dir"
    dest_dir="$TAR_ROOT/plugins/$plugin_dir"
    
    if [[ -d "$src_dir" ]]; then
        echo "复制插件目录: $plugin_dir"
        mkdir -p "$dest_dir"
        cp -r "$src_dir/"* "$dest_dir/" 2>/dev/null || true
    fi
done

# 复制 QML 模块
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

echo "4. 设置 RPATH..."

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
MAIN_EXECUTABLE="$TAR_ROOT/XFileTransit"
if [[ -f "$MAIN_EXECUTABLE" ]]; then
    set_rpath "$MAIN_EXECUTABLE" '$ORIGIN/lib'
fi

# 设置 Qt 插件的 RPATH
find "$TAR_ROOT/plugins" -name "*.so" -type f 2>/dev/null | while read -r plugin; do
    set_rpath "$plugin" '$ORIGIN/../../lib'
done

# 设置 Qt 库的 RPATH
find "$TAR_ROOT/lib" -name "libQt6*.so*" -type f 2>/dev/null | while read -r qt_lib; do
    set_rpath "$qt_lib" '$ORIGIN'
done

echo "5. 创建 AppImage 配置文件..."

# 5.1 创建桌面入口文件
DESKTOP_FILE="${APP_DIR}/${APP_NAME}.desktop"
cat > "$DESKTOP_FILE" << 'EOF'
[Desktop Entry]
Type=Application
Name=XFileTransit
Comment=File Transfer Application
Exec=XFileTransit
Icon=XFileTransit
Categories=Utility;
StartupNotify=false
Terminal=false
EOF

# 5.2 处理图标
if [[ "$ICON_FOUND" == true ]] && [[ -f "$ICON_PATH" ]]; then
    mkdir -p "${APP_DIR}/usr/share/icons/hicolor/512x512/apps/"
    if command -v convert &> /dev/null; then
        convert "$ICON_PATH" -resize 512x512 "${APP_DIR}/usr/share/icons/hicolor/512x512/apps/${APP_NAME}.png"
    else
        cp "$ICON_PATH" "${APP_DIR}/usr/share/icons/hicolor/512x512/apps/${APP_NAME}.png"
    fi
    cp "${APP_DIR}/usr/share/icons/hicolor/512x512/apps/${APP_NAME}.png" "${APP_DIR}/${APP_NAME}.png"
else
    echo "警告: 未找到图标文件，将使用默认占位符"
fi

# 5.3 创建 AppRun 启动脚本
cat > "${APP_DIR}/AppRun" << 'EOF'
#!/bin/bash

# 设置库路径
HERE="$(dirname "$(readlink -f "$0")")"
export PATH="${HERE}/usr/bin:${PATH}"
export LD_LIBRARY_PATH="${HERE}/usr/lib:${LD_LIBRARY_PATH}"

# 设置 Qt 插件路径
export QT_PLUGIN_PATH="${HERE}/usr/plugins"
export QML2_IMPORT_PATH="${HERE}/usr/qml"

# 运行应用程序
exec "${HERE}/usr/XFileTransit" "$@"
EOF
chmod +x "${APP_DIR}/AppRun"

mkdir -p "${APP_DIR}/usr/bin"
ln -sf "../XFileTransit" "${APP_DIR}/usr/bin/XFileTransit"

# 检查是否已安装 appimagetool
echo "6. 使用 appimagetool 打包..."

function check_appimagetool() {
    if command -v appimagetool &> /dev/null; then
        if appimagetool --version &> /dev/null; then
            return 0
        fi
    fi
    return 1
}

if check_appimagetool; then
    echo "使用系统中已安装的 appimagetool"
    APPIMAGE_TOOL="appimagetool"
else
    echo "appimagetool 不可用或未安装，正在下载/修复..."
    
    APPIMAGE_TOOL_URL="https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage"
    
    # 下载到临时目录
    TEMP_TOOL="/tmp/appimagetool-x86_64.AppImage"
    
    if [[ ! -f "$TEMP_TOOL" ]]; then
        rm -f $TEMP_TOOL
        echo "下载 appimagetool 到临时目录..."
        wget -O "$TEMP_TOOL" "$APPIMAGE_TOOL_URL" || {
            echo "错误: 下载 appimagetool 失败"
            exit 1
        }
        chmod +x "$TEMP_TOOL"
    fi
    
    # 验证下载的工具是否可用
    if "$TEMP_TOOL" --version &> /dev/null; then
        echo "临时目录的 appimagetool 可用"
        APPIMAGE_TOOL="$TEMP_TOOL"
        
        echo "安装 appimagetool 到 /usr/local/bin..."
        sudo cp "$TEMP_TOOL" /usr/local/bin/appimagetool
        sudo chmod +x /usr/local/bin/appimagetool
        APPIMAGE_TOOL="appimagetool"
    else
        echo "错误: 下载的 appimagetool 无法运行"
        exit 1
    fi
fi

# 打包 AppImage
echo "正在生成 AppImage..."
FINAL_NAME="${APP_NAME}-${VERSION}-${ARCH}.AppImage"
FINAL_PATH="${WORK_DIR}/${FINAL_NAME}"
$APPIMAGE_TOOL "$APP_DIR" "$FINAL_PATH" --no-appstream

echo "7. 打包完成！"

if [[ -f "$FINAL_PATH" ]]; then
    echo "======================================"
    echo "AppImage 创建成功!"
    echo "文件: $FINAL_PATH"
    echo "大小: $(du -h "$FINAL_PATH" | awk '{print $1}')"
    echo ""
    echo "使用说明:"
    echo "1. 给予执行权限: chmod +x $FINAL_PATH"
    echo "2. 直接运行: $FINAL_PATH"
    echo "======================================"
else
    echo "错误: AppImage 文件未生成"
    exit 1
fi
