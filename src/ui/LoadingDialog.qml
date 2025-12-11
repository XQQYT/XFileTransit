import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Popup {
    id: loadingDialog
    width: 280
    height: 180
    modal: false
    dim: true
    focus: false
    closePolicy: Popup.NoAutoClose
    
    // 公共接口
    property string message: qsTr("加载中...")
    property string buttonText: qsTr("取消")
    property bool autoCenter: true
    
    property color windowBg: "#ffffff"
    property color windowInnerBg: "#f9fafb"
    property color windowBorder: "#e5e7eb"
    property int windowRadius: 16
    
    property color topBarGradientStart: "#6366f1"
    property color topBarGradientMiddle: "#8b5cf6"
    property color topBarGradientEnd: "#a855f7"
    
    property color spinnerOuterBg: "#f5f3ff"
    property color spinnerOuterBorder: "#ede9fe"
    property color spinnerInnerBorder: "#6366f1"
    property color spinnerCenter: "#6366f1"
    property color spinnerGradientStart: "#6366f1"
    property color spinnerGradientEnd: "#8b5cf6"
    
    property color textPrimary: "#1f2937"
    property color textSecondary: "#6b7280"
    property color textAccent: "#4f46e5"
    property color textDisabled: "#9ca3af"
    
    property color buttonBg: "#ffffff"
    property color buttonBgHover: "#f8fafc"
    property color buttonBgPressed: "#f3f4f6"
    property color buttonBorder: "#d1d5db"
    property color buttonBorderHover: "#6366f1"
    property color buttonTextColor: "#6b7280"
    property color buttonTextHover: "#4f46e5"
    property int buttonRadius: 18
    
    // 主题切换函数
    function setTheme(theme_index) {
        switch(theme_index)
        {
            case 0:
                //浅色主题
                windowBg = "#ffffff"
                windowInnerBg = "#f9fafb"
                windowBorder = "#e5e7eb"
                
                topBarGradientStart = "#6366f1"
                topBarGradientMiddle = "#8b5cf6"
                topBarGradientEnd = "#a855f7"
                
                spinnerOuterBg = "#f5f3ff"
                spinnerOuterBorder = "#ede9fe"
                spinnerInnerBorder = "#6366f1"
                spinnerCenter = "#6366f1"
                spinnerGradientStart = "#6366f1"
                spinnerGradientEnd = "#8b5cf6"
                
                textPrimary = "#1f2937"
                textSecondary = "#6b7280"
                textAccent = "#4f46e5"
                textDisabled = "#9ca3af"
                
                buttonBg = "#ffffff"
                buttonBgHover = "#f8fafc"
                buttonBgPressed = "#f3f4f6"
                buttonBorder = "#d1d5db"
                buttonBorderHover = "#6366f1"
                buttonTextColor = "#6b7280"
                buttonTextHover = "#4f46e5"
                break
            case 1:
                //深色主题
                windowBg = "#1f2937"
                windowInnerBg = "#111827"
                windowBorder = "#374151"
                
                topBarGradientStart = "#6366f1"
                topBarGradientMiddle = "#8b5cf6"
                topBarGradientEnd = "#a855f7"
                
                spinnerOuterBg = "#1e1b4b"
                spinnerOuterBorder = "#312e81"
                spinnerInnerBorder = "#6366f1"
                spinnerCenter = "#6366f1"
                spinnerGradientStart = "#6366f1"
                spinnerGradientEnd = "#8b5cf6"
                
                textPrimary = "#f9fafb"
                textSecondary = "#d1d5db"
                textAccent = "#a78bfa"
                textDisabled = "#6b7280"
                
                buttonBg = "#374151"
                buttonBgHover = "#4b5563"
                buttonBgPressed = "#6b7280"
                buttonBorder = "#4b5563"
                buttonBorderHover = "#6366f1"
                buttonTextColor = "#d1d5db"
                buttonTextHover = "#a78bfa"
                break
            default:
                return
        }
    }
    
    // 信号
    signal buttonClicked()
    
    onOpened: {
        if (autoCenter && parent) {
            x = (parent.width - width) / 2
            y = (parent.height - height) / 2
        }
    }
    
    // 背景
    background: Rectangle {
        color: windowBg
        radius: windowRadius
        border.color: windowBorder
        border.width: 1
        
        // 浅色背景
        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: windowInnerBg
            opacity: 0.5
        }
        
        // 顶部装饰条
        Rectangle {
            width: parent.width
            height: 4
            radius: 2
            anchors.top: parent.top
            gradient: Gradient {
                GradientStop { position: 0.0; color: topBarGradientStart }
                GradientStop { position: 0.5; color: topBarGradientMiddle }
                GradientStop { position: 1.0; color: topBarGradientEnd }
            }
        }
    }
    
    contentItem: ColumnLayout {
        spacing: 20
        anchors.centerIn: parent
        
        // 旋转加载器
        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            width: 48
            height: 48
            radius: 24
            color: spinnerOuterBg
            border.width: 3
            border.color: spinnerOuterBorder
            
            // 旋转部分
            Rectangle {
                anchors.centerIn: parent
                width: 32
                height: 32
                radius: 16
                color: "transparent"
                border.width: 3
                border.color: spinnerInnerBorder
                
                RotationAnimation on rotation {
                    from: 0
                    to: 360
                    duration: 1500
                    loops: Animation.Infinite
                    running: loadingDialog.visible
                }
                
                // 渐变色覆盖
                Rectangle {
                    width: parent.width
                    height: 6
                    anchors.centerIn: parent
                    rotation: 45
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: spinnerGradientStart }
                        GradientStop { position: 1.0; color: spinnerGradientEnd }
                    }
                }
            }
            
            // 中心点
            Rectangle {
                anchors.centerIn: parent
                width: 8
                height: 8
                radius: 4
                color: spinnerCenter
            }
        }
        
        // 消息文本
        ColumnLayout {
            spacing: 4
            Layout.alignment: Qt.AlignHCenter
            
            Text {
                text: loadingDialog.message
                font.pixelSize: 16
                font.family: "Microsoft YaHei UI"
                font.weight: Font.Medium
                color: textPrimary
                horizontalAlignment: Text.AlignHCenter
            }
            
            // 加载中动画文本
            Text {
                id: loadingText
                visible: loadingDialog.message === qsTr("加载中...")
                text: {
                    var dots = ""
                    var dotCount = (Math.floor(Date.now() / 500) % 4)
                    for (var i = 0; i < dotCount; i++) {
                        dots += "."
                    }
                    return dots
                }
                font.pixelSize: 14
                font.family: "Microsoft YaHei UI"
                color: textSecondary
                
                Timer {
                    interval: 500
                    running: loadingText.visible
                    repeat: true
                    onTriggered: loadingText.textChanged()
                }
            }
        }
        
        // 取消按钮
        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 12
            width: 100
            height: 36
            radius: buttonRadius
            color: cancelMouse.pressed ? buttonBgPressed : 
                   cancelMouse.containsMouse ? buttonBgHover : buttonBg
            border.color: cancelMouse.containsMouse ? buttonBorderHover : buttonBorder
            border.width: 1.5
            
            Text {
                anchors.centerIn: parent
                text: loadingDialog.buttonText
                font.pixelSize: 14
                font.family: "Microsoft YaHei UI"
                font.weight: Font.Medium
                color: cancelMouse.containsMouse ? buttonTextHover : buttonTextColor
            }
            
            MouseArea {
                id: cancelMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    loadingDialog.buttonClicked()
                    loadingDialog.close()
                }
            }
        }
    }
    
    // 显示对话框
    function show(msg, btnText) {
        if (msg !== undefined) message = msg
        if (btnText !== undefined) buttonText = btnText
        open()
    }
    
    // 隐藏对话框
    function hide() {
        close()
    }
}