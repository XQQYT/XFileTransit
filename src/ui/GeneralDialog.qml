import QtQuick
import QtQuick.Window
import QtQuick.Layouts

Window {
    id: root
    width: 400
    height: 180
    modality: Qt.ApplicationModal
    flags: Qt.FramelessWindowHint | Qt.Dialog
    color: "transparent"
    visible: false
    
    property string title: qsTr("提示")
    property string text: ""
    property int buttons: root.ok
    property int iconType: root.info
    
    readonly property int ok: 0x0001        // 1
    readonly property int cancel: 0x0002    // 2
    readonly property int yes: 0x0004       // 4
    readonly property int no: 0x0008        // 8
    readonly property int closeWin: 0x0010  // 16
    readonly property int hideWin: 0x0020   // 32
    
    // 图标类型枚举
    readonly property int none: 0
    readonly property int info: 1
    readonly property int success: 2
    readonly property int error: 3
    readonly property int warning: 4
    
    property color windowBg: "#ffffff"
    property color windowBorder: "#e5e7eb"
    property int windowRadius: 8
    
    property color textPrimary: "#111827"
    property color textSecondary: "#6b7280"
    property color textOnPrimary: "white"
    
    property color cancelButtonBg: "#f9fafb"
    property color cancelButtonBgHover: "#f3f4f6"
    property color cancelButtonBorder: "#d1d5db"
    property color cancelButtonText: "#374151"
    
    property color closeButtonBg: "#6b7280"
    property color closeButtonBgHover: "#4b5563"
    property color closeButtonText: "white"
    
    property color successIcon: "#10b981"
    property color errorIcon: "#ef4444"
    property color warningIcon: "#f59e0b"
    property color infoIcon: "#3b82f6"
    property color defaultIcon: "#6b7280"
    
    property int buttonRadius: 4
    
    // 主题切换函数
    function setTheme(theme_index) {
        switch(theme_index)
        {
            case 0:
                //浅色主题
                windowBg = "#ffffff"
                windowBorder = "#e5e7eb"
                
                textPrimary = "#111827"
                textSecondary = "#6b7280"
                
                cancelButtonBg = "#f9fafb"
                cancelButtonBgHover = "#f3f4f6"
                cancelButtonBorder = "#d1d5db"
                cancelButtonText = "#374151"
                
                closeButtonBg = "#6b7280"
                closeButtonBgHover = "#4b5563"
                closeButtonText = "white"
                
                successIcon = "#10b981"
                errorIcon = "#ef4444"
                warningIcon = "#f59e0b"
                infoIcon = "#3b82f6"
                defaultIcon = "#6b7280"
                break
            case 1:
                //深色主题
                windowBg = "#1f2937"
                windowBorder = "#374151"
                
                textPrimary = "#f9fafb"
                textSecondary = "#d1d5db"
                
                cancelButtonBg = "#374151"
                cancelButtonBgHover = "#4b5563"
                cancelButtonBorder = "#4b5563"
                cancelButtonText = "#d1d5db"
                
                closeButtonBg = "#6b7280"
                closeButtonBgHover = "#4b5563"
                closeButtonText = "white"
                
                successIcon = "#10b981"
                errorIcon = "#ef4444"
                warningIcon = "#f59e0b"
                infoIcon = "#3b82f6"
                defaultIcon = "#6b7280"
                break
            default:
                return
        }
    }
    
    // 信号
    signal accepted()
    signal rejected()
    signal clicked(int button)
    
    // 组件加载完成后设置初始位置
    Component.onCompleted: {
        centerOnScreen()
    }
    
    // 窗口显示时重新居中
    onVisibleChanged: {
        if (visible) {
            centerOnScreen()
        }
    }
    
    function centerOnScreen() {
        Qt.callLater(function() {
            var screen = getCurrentScreen()
            if (!screen) {
                console.error("无法获取屏幕对象")
                return
            }
            
            var screenWidth = screen.width || screen.desktopAvailableWidth
            var screenHeight = screen.height || screen.desktopAvailableHeight
            
            if (screenWidth <= 0 || screenHeight <= 0) {
                console.warn("屏幕尺寸无效，使用默认值")
                screenWidth = 1920
                screenHeight = 1080
            }
            
            // 计算居中位置
            var centerX = screen.virtualX + (screenWidth - root.width) / 2
            var centerY = screen.virtualY + (screenHeight - root.height) / 2
            
            // 设置窗口位置
            root.x = centerX
            root.y = centerY
        })
    }
    
    // 获取当前屏幕
    function getCurrentScreen() {
        if (Window.window && Window.window.screen) {
            return Window.window.screen
        } else if (Qt.application.screens.length > 0) {
            return Qt.application.screens[0]
        } else if (Screen) {
            return Screen
        }
        return null
    }
    
    // 外部调用接口
    function showDialog(title, text, iconType, buttons) {
        root.title = title || qsTr("提示")
        root.text = text || ""
        root.iconType = iconType || root.info
        root.buttons = buttons || root.ok
        
        // 居中并显示
        centerOnScreen()
        root.show()
        root.requestActivate()
        root.raise()
    }
    
    function closeDialog() {
        root.close()
    }

    // 对话框主体
    Rectangle {
        anchors.centerIn: parent
        width: 360
        height: 160
        radius: windowRadius
        color: windowBg
        border.color: windowBorder
        border.width: 1
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 16
            
            // 标题行
            RowLayout {
                spacing: 12
                
                // 图标
                Rectangle {
                    width: 32
                    height: 32
                    radius: 6
                    color: getIconColor()
                    
                    Text {
                        anchors.centerIn: parent
                        text: getIconText()
                        font.pixelSize: 16
                        color: textOnPrimary
                        font.bold: true
                    }
                }
                
                // 标题
                Text {
                    text: root.title
                    font.pixelSize: 16
                    font.bold: true
                    color: textPrimary
                    Layout.fillWidth: true
                }
            }
            
            // 消息内容
            Text {
                text: root.text
                font.pixelSize: 18
                color: textSecondary
                wrapMode: Text.Wrap
                lineHeight: 1.4
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
            
            // 按钮区域
            Row {
                spacing: 8
                Layout.alignment: Qt.AlignRight
                
                // 取消按钮
                Rectangle {
                    width: 72
                    height: 32
                    radius: buttonRadius
                    color: cancelArea.containsMouse ? cancelButtonBgHover : cancelButtonBg
                    border.color: cancelButtonBorder
                    border.width: 1
                    visible: (root.buttons & root.cancel) || (root.buttons & root.no)
                    
                    Text {
                        anchors.centerIn: parent
                        text: (root.buttons & root.cancel) ? qsTr("取消") : qsTr("否")
                        font.pixelSize: 13
                        color: cancelButtonText
                    }
                    
                    MouseArea {
                        id: cancelArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            root.clicked((root.buttons & root.cancel) ? root.cancel : root.no)
                            root.rejected()
                            root.close()
                        }
                    }
                }
                
                // 确定按钮
                Rectangle {
                    width: 72
                    height: 32
                    radius: buttonRadius
                    color: okArea.containsMouse ? Qt.darker(getIconColor(), 1.1) : getIconColor()
                    visible: (root.buttons & root.yes) || (root.buttons & root.ok)
                    
                    Text {
                        anchors.centerIn: parent
                        text: (root.buttons & root.yes) ? qsTr("是") : qsTr("确定")
                        font.pixelSize: 13
                        color: textOnPrimary
                    }
                    
                    MouseArea {
                        id: okArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            root.clicked((root.buttons & root.yes) ? root.yes : root.ok)
                            root.accepted()
                            root.close()
                        }
                    }
                }
                
                // 隐藏按钮
                Rectangle {
                    width: 120
                    height: 32
                    radius: buttonRadius
                    color: hideArea.containsMouse ? Qt.darker(getIconColor(), 1.1) : getIconColor()
                    visible: root.buttons & root.hideWin
                    
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("最小化到托盘")
                        font.pixelSize: 13
                        color: textOnPrimary
                    }
                    
                    MouseArea {
                        id: hideArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            root.clicked(root.hideWin)
                            root.rejected()
                            root.close()
                        }
                    }
                }
                
                // 关闭按钮
                Rectangle {
                    width: 72
                    height: 32
                    radius: buttonRadius
                    color: closeArea.containsMouse ? closeButtonBgHover : closeButtonBg
                    visible: root.buttons & root.closeWin
                    
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("关闭")
                        font.pixelSize: 13
                        color: closeButtonText
                    }
                    
                    MouseArea {
                        id: closeArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            root.clicked(root.closeWin)
                            root.accepted()
                            root.close()
                        }
                    }
                }
            }
        }
    }
    
    function getIconColor() {
        switch(root.iconType) {
            case root.success: return successIcon
            case root.error: return errorIcon
            case root.warning: return warningIcon
            case root.info: return infoIcon
            default: return defaultIcon
        }
    }
    
    function getIconText() {
        switch(root.iconType) {
            case root.success: return "✓"
            case root.error: return "✕"
            case root.warning: return "!"
            case root.info: return "i"
            default: return ""
        }
    }
}