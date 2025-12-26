import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

Window {
    id: root
    width: 400
    height: contentContainer.height
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
    readonly property int cancelTransit: 0x0040   // 64
    
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
    
    // 当text或title改变时，重新计算大小
    onTextChanged: Qt.callLater(root.adjustWindowSize)
    onTitleChanged: Qt.callLater(root.adjustWindowSize)
    
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
        
        // 调整窗口大小以适应内容
        adjustWindowSize()
        
        // 居中并显示
        centerOnScreen()
        root.show()
        root.requestActivate()
        root.raise()
    }
    
    function closeDialog() {
        root.close()
    }
    
    // 调整窗口大小以适应内容
    function adjustWindowSize() {
        if (!contentContainer.visible) return
        
        // 计算文本所需的高度
        var textHeight = messageText.contentHeight
        var textWidth = messageText.contentWidth
        
        // 计算合适的宽度
        var maxTextWidth = Math.max(280, Math.min(500, textWidth + 80))
        var containerWidth = Math.max(320, Math.min(600, maxTextWidth))
        
        // 计算合适的高度
        var maxTextHeight = Math.min(400, textHeight) // 限制最大文本高度
        var containerHeight = Math.max(140, Math.min(500, 
            maxTextHeight + 120 + buttonArea.height)) // 120=标题区+边距
        
        // 设置容器大小
        contentContainer.width = containerWidth
        contentContainer.height = containerHeight
        
        // 设置窗口大小
        root.width = contentContainer.width
        root.height = contentContainer.height
    }

    // 对话框主体容器
    Rectangle {
        id: contentContainer
        anchors.centerIn: parent
        radius: windowRadius
        color: windowBg
        border.color: windowBorder
        border.width: 1
        
        ColumnLayout {
            id: mainLayout
            anchors.fill: parent
            anchors.margins: 20
            spacing: 16
            
            // 标题行
            RowLayout {
                spacing: 12
                Layout.fillWidth: true
                
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
                    wrapMode: Text.Wrap
                    elide: Text.ElideRight
                }
            }
            
            // 消息内容区域
            ScrollView {
                id: scrollView
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredHeight: Math.min(400, messageText.contentHeight + 20)
                clip: true
                
                TextArea {
                    id: messageText
                    width: scrollView.width - 20
                    text: root.text
                    font.pixelSize: getFontSize()
                    color: textSecondary
                    wrapMode: Text.Wrap
                    readOnly: true
                    selectByMouse: false
                    background: Rectangle {
                        color: "transparent"
                    }
                    
                    // 计算合适的字体大小
                    function getFontSize() {
                        var lines = text.split('\n').length
                        var charsPerLine = text.length / Math.max(1, lines)
                        
                        if (lines > 15 || charsPerLine > 80) {
                            return 12
                        } else if (lines > 10 || charsPerLine > 60) {
                            return 13
                        } else {
                            return 14
                        }
                    }
                    
                    // 文本改变时调整
                    onTextChanged: {
                        Qt.callLater(root.adjustWindowSize)
                    }
                    
                    // 内容大小改变时调整
                    onContentHeightChanged: {
                        Qt.callLater(root.adjustWindowSize)
                    }
                }
            }
            
            // 按钮区域
            Row {
                id: buttonArea
                spacing: 8
                Layout.alignment: Qt.AlignRight | Qt.AlignBottom
                
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
                    width: 72
                    height: 32
                    radius: buttonRadius
                    color: hideArea.containsMouse ? Qt.darker(getIconColor(), 1.1) : getIconColor()
                    visible: root.buttons & root.hideWin
                    
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("最小化")
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

                // 取消传输按钮
                Rectangle {
                    width: 72
                    height: 32
                    radius: buttonRadius
                    color: cancelTransitArea.containsMouse ? closeButtonBgHover : closeButtonBg
                    visible: root.buttons & root.cancelTransit
                    
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("取消传输")
                        font.pixelSize: 13
                        color: textOnPrimary
                    }
                    
                    MouseArea {
                        id: cancelTransitArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            root.clicked(root.cancelTransit)
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