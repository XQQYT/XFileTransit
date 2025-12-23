import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: loadingDialog
    width: 280
    height: contentLayout.implicitHeight + 40  // 动态高度
    modal: false
    dim: true
    focus: false
    closePolicy: Popup.NoAutoClose
    
    property bool enableBtn: true
    property bool enableProgress: false
    // 公共接口
    property string message: qsTr("加载中...")
    property string buttonText: qsTr("取消")
    property bool autoCenter: true
    
    property real progress: 0.0
    property string progressText: "0%"
    property int progressPrecision: 1
    
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
    
    // 进度条相关颜色
    property color progressTrackColor: "#e5e7eb"
    property color progressFillColor: "#6366f1"
    property color progressTextColor: "#4f46e5"
    property color progressBackground: "#f9fafb"
    
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
                
                // 进度条颜色
                progressTrackColor = "#e5e7eb"
                progressFillColor = "#6366f1"
                progressTextColor = "#4f46e5"
                progressBackground = "#f9fafb"
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
                
                // 进度条颜色
                progressTrackColor = "#4b5563"
                progressFillColor = "#8b5cf6"
                progressTextColor = "#a78bfa"
                progressBackground = "#1f2937"
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
    
    // 进度改变时更新文本
    onProgressChanged: {
        var percent = progress * 100;
        progressText = percent.toFixed(progressPrecision) + "%";
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
        id: contentLayout
        spacing: 16  // 减小间距
        anchors.fill: parent
        anchors.margins: 20  // 增加内边距
        
        // 旋转加载器或进度条
        Item {
            Layout.alignment: Qt.AlignHCenter
            width: 80
            height: 80
            
            // 旋转加载器（当enableProgress为false时显示）
            Rectangle {
                id: spinner
                anchors.fill: parent
                radius: 40
                color: spinnerOuterBg
                border.width: 4
                border.color: spinnerOuterBorder
                visible: !enableProgress
                
                // 旋转部分
                Rectangle {
                    anchors.centerIn: parent
                    width: 50
                    height: 50
                    radius: 25
                    color: "transparent"
                    border.width: 4
                    border.color: spinnerInnerBorder
                    
                    RotationAnimation on rotation {
                        from: 0
                        to: 360
                        duration: 1500
                        loops: Animation.Infinite
                        running: loadingDialog.visible && !enableProgress
                    }
                    
                    // 渐变色覆盖
                    Rectangle {
                        width: parent.width
                        height: 8
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
                    width: 10
                    height: 10
                    radius: 5
                    color: spinnerCenter
                }
            }
            
            // 进度环（当enableProgress为true时显示）
            Rectangle {
                id: progressCircle
                anchors.fill: parent
                radius: 40
                color: progressBackground
                border.width: 4
                border.color: progressTrackColor
                visible: enableProgress
                
                // 进度填充
                Canvas {
                    id: progressCanvas
                    anchors.fill: parent
                    antialiasing: true
                    
                    onPaint: {
                        var ctx = getContext("2d");
                        ctx.reset();
                        
                        // 绘制背景圆
                        ctx.beginPath();
                        ctx.lineWidth = 4;
                        ctx.strokeStyle = progressTrackColor;
                        ctx.arc(width/2, height/2, width/2 - 6, 0, Math.PI * 2);
                        ctx.stroke();
                        
                        // 绘制进度弧
                        ctx.beginPath();
                        ctx.lineWidth = 4;
                        ctx.lineCap = "round";
                        ctx.strokeStyle = progressFillColor;
                        var startAngle = -Math.PI / 2; // 从顶部开始
                        var endAngle = startAngle + (loadingDialog.progress * Math.PI * 2);
                        ctx.arc(width/2, height/2, width/2 - 6, startAngle, endAngle);
                        ctx.stroke();
                    }
                }
                
                // 进度文本（居中显示）
                Text {
                    anchors.centerIn: parent
                    text: loadingDialog.progressText
                    font.pixelSize: 16
                    font.family: "Microsoft YaHei UI"
                    font.weight: Font.Bold
                    color: progressTextColor
                }
                
                // 当进度改变时重绘画布
                Connections {
                    target: loadingDialog
                    function onProgressChanged() {
                        progressCanvas.requestPaint();
                    }
                }
            }
        }
        
        // 消息文本（只在非进度模式显示）
        ColumnLayout {
            spacing: 4
            Layout.alignment: Qt.AlignHCenter
            visible: !enableProgress
            Layout.preferredHeight: visible ? implicitHeight : 0
            
            Text {
                text: loadingDialog.message
                font.pixelSize: 16
                font.family: "Microsoft YaHei UI"
                font.weight: Font.Medium
                color: textPrimary
                horizontalAlignment: Text.AlignHCenter
            }
            
            // 加载中动画文本（只在普通加载模式且消息为默认值时显示）
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
            width: 100
            height: 36
            radius: buttonRadius
            color: cancelMouse.pressed ? buttonBgPressed : 
                   cancelMouse.containsMouse ? buttonBgHover : buttonBg
            border.color: cancelMouse.containsMouse ? buttonBorderHover : buttonBorder
            border.width: 1.5
            visible: enableBtn
            
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
    function show(msg, btnText, progressValue) {
        if (msg !== undefined) message = msg
        if (btnText !== undefined) buttonText = btnText
        if (progressValue !== undefined) {
            enableProgress = true
            progress = Math.max(0.0, Math.min(1.0, progressValue))
        } else {
            enableProgress = false
        }
        open()
    }
    
    // 更新进度（仅当启用进度模式时有效）
    function updateProgress(value) {
        if (enableProgress) {
            progress = Math.max(0.0, Math.min(1.0, value))
        }
    }
    
    // 设置进度小数位数
    function setProgressPrecision(precision) {
        progressPrecision = Math.max(0, Math.min(3, precision))
    }
    
    // 隐藏对话框
    function hide() {
        close()
    }
}