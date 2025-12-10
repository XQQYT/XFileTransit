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
        color: "#ffffff"
        radius: 16
        border.color: "#e5e7eb"
        border.width: 1
        
        // 浅色背景
        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: "#f9fafb"
            opacity: 0.5
        }
        
        // 顶部装饰条
        Rectangle {
            width: parent.width
            height: 4
            radius: 2
            anchors.top: parent.top
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#6366f1" }
                GradientStop { position: 0.5; color: "#8b5cf6" }
                GradientStop { position: 1.0; color: "#a855f7" }
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
            color: "#f5f3ff"
            border.width: 3
            border.color: "#ede9fe"
            
            // 旋转部分
            Rectangle {
                anchors.centerIn: parent
                width: 32
                height: 32
                radius: 16
                color: "transparent"
                border.width: 3
                border.color: "#6366f1"
                
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
                        GradientStop { position: 0.0; color: "#6366f1" }
                        GradientStop { position: 1.0; color: "#8b5cf6" }
                    }
                }
            }
            
            // 中心点
            Rectangle {
                anchors.centerIn: parent
                width: 8
                height: 8
                radius: 4
                color: "#6366f1"
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
                color: "#1f2937"
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
                color: "#6b7280"
                
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
            radius: 18
            color: cancelMouse.containsPress ? "#f3f4f6" : 
                   cancelMouse.containsMouse ? "#f8fafc" : "#ffffff"
            border.color: cancelMouse.containsMouse ? "#6366f1" : "#d1d5db"
            border.width: 1.5
            
            Text {
                anchors.centerIn: parent
                text: loadingDialog.buttonText
                font.pixelSize: 14
                font.family: "Microsoft YaHei UI"
                font.weight: Font.Medium
                color: cancelMouse.containsMouse ? "#4f46e5" : "#6b7280"
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