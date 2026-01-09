import QtQuick
import QtQuick.Window
import QtQuick.Layouts

Window {
    id: connectionDialog
    width: 500
    height: 350
    modality: Qt.ApplicationModal
    flags: Qt.FramelessWindowHint | Qt.Dialog
    color: "transparent"
    visible: false
    
    property string device_ip: ""
    property string device_name: ""
    property var connection_model: null
    
    // 主题颜色变量
    property color windowBg: "#ffffff"
    property color windowBorder: "#f0f0f0"
    
    property color textPrimary: "#1f2937"
    property color textSecondary: "#6b7280"
    property color textTertiary: "#9ca3af"
    property color textOnPrimary: "white"
    
    property color fieldBg: "#f8fafc"
    property color fieldBorder: "#e2e8f0"
    property color fieldText: "#1e293b"
    
    property color rejectButtonBg: "#fafafa"
    property color rejectButtonBgHover: "#fef2f2"
    property color rejectButtonBorder: "#e5e7eb"
    property color rejectButtonBorderHover: "#fca5a5"
    property color rejectButtonText: "#6b7280"
    property color rejectButtonTextHover: "#dc2626"
    
    property color acceptButtonBg: "#6366f1"
    property color acceptButtonBgHover: "#4f46e5"
    property color acceptButtonText: "white"
    
    property int borderRadius: 14
    property int fieldRadius: 8
    property int buttonRadius: 8
    
    // 主题切换函数
    function setTheme(theme_index) {
        switch(theme_index)
        {
            case 0:
                //浅色主题
                windowBg = "#ffffff"
                windowBorder = "#f0f0f0"
                
                textPrimary = "#1f2937"
                textSecondary = "#6b7280"
                textTertiary = "#9ca3af"
                
                fieldBg = "#f8fafc"
                fieldBorder = "#e2e8f0"
                fieldText = "#1e293b"
                
                rejectButtonBg = "#fafafa"
                rejectButtonBgHover = "#fef2f2"
                rejectButtonBorder = "#e5e7eb"
                rejectButtonBorderHover = "#fca5a5"
                rejectButtonText = "#6b7280"
                rejectButtonTextHover = "#dc2626"
                
                acceptButtonBg = "#6366f1"
                acceptButtonBgHover = "#4f46e5"
                break
            case 1:
                //深色主题
                windowBg = "#1f2937"
                windowBorder = "#374151"
                
                textPrimary = "#f9fafb"
                textSecondary = "#d1d5db"
                textTertiary = "#9ca3af"
                
                fieldBg = "#111827"
                fieldBorder = "#374151"
                fieldText = "#f3f4f6"
                
                rejectButtonBg = "#374151"
                rejectButtonBgHover = "#7f1d1d"
                rejectButtonBorder = "#4b5563"
                rejectButtonBorderHover = "#991b1b"
                rejectButtonText = "#d1d5db"
                rejectButtonTextHover = "#fca5a5"
                
                acceptButtonBg = "#6366f1"
                acceptButtonBgHover = "#4f46e5"
                break
            default:
                return
        }
    }
    
    signal accepted(string ip, string name)
    signal rejected(string ip, string name)
    
    // 居中
    x: (Screen.width - width) / 2
    y: (Screen.height - height) / 2
    
    onVisibleChanged: if (visible) requestActivate()
    
    function showDialog(ip, name, model) {
        device_ip = ip
        device_name = name
        connection_model = model
        show()
    }

    Rectangle {
        anchors.centerIn: parent
        width: 500
        height: 350
        radius: borderRadius
        color: windowBg
        border.color: windowBorder
        border.width: 1
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 20
            
            // 标题
            Text {
                text: qsTr("连接请求")
                font.pixelSize: 20
                font.bold: true
                font.family: "Microsoft YaHei UI"
                color: textPrimary
                Layout.alignment: Qt.AlignHCenter
            }
            
            // 设备信息区域
            ColumnLayout {
                spacing: 16
                Layout.fillWidth: true
                Layout.fillHeight: true
                
                // IP地址
                ColumnLayout {
                    spacing: 4
                    
                    Text {
                        text: qsTr("IP地址/设备代码")
                        font.pixelSize: 13
                        color: textSecondary
                        font.weight: Font.Medium
                    }
                    
                    Rectangle {
                        Layout.fillWidth: true
                        height: 40
                        radius: fieldRadius
                        color: fieldBg
                        border.color: fieldBorder
                        border.width: 1
                        
                        Text {
                            anchors.centerIn: parent
                            text: device_ip
                            font.pixelSize: 15
                            color: fieldText
                            font.bold: true
                        }
                    }
                }
                
                // 设备名称
                ColumnLayout {
                    spacing: 4
                    
                    Text {
                        text: qsTr("设备名称")
                        font.pixelSize: 13
                        color: textSecondary
                        font.weight: Font.Medium
                    }
                    
                    Rectangle {
                        Layout.fillWidth: true
                        height: 40
                        radius: fieldRadius
                        color: fieldBg
                        border.color: fieldBorder
                        border.width: 1
                        
                        Text {
                            anchors.centerIn: parent
                            text: device_name
                            font.pixelSize: 15
                            color: fieldText
                            font.bold: true
                        }
                    }
                }
            }
            
            // 提示文字
            Text {
                text: qsTr("是否允许此设备连接到您的计算机？")
                font.pixelSize: 13
                color: textTertiary
                Layout.alignment: Qt.AlignHCenter
            }
            
            // 按钮区域
            Row {
                spacing: 12
                Layout.alignment: Qt.AlignHCenter
                
                // 拒绝按钮
                Rectangle {
                    width: 90
                    height: 38
                    radius: buttonRadius
                    color: rejectMouse.containsMouse ? rejectButtonBgHover : rejectButtonBg
                    border.color: rejectMouse.containsMouse ? rejectButtonBorderHover : rejectButtonBorder
                    border.width: 1.5
                    
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("拒绝")
                        font.pixelSize: 14
                        color: rejectMouse.containsMouse ? rejectButtonTextHover : rejectButtonText
                        font.weight: Font.Medium
                    }
                    
                    MouseArea {
                        id: rejectMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            connectionDialog.rejected(device_ip, device_name)
                            if (connection_model) connection_model.rejected(device_ip, device_name)
                            connectionDialog.close()
                        }
                    }
                }
                
                // 接受按钮
                Rectangle {
                    width: 90
                    height: 38
                    radius: buttonRadius
                    color: acceptMouse.containsMouse ? acceptButtonBgHover : acceptButtonBg
                    
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("接受")
                        font.pixelSize: 14
                        color: textOnPrimary
                        font.weight: Font.Medium
                    }
                    
                    MouseArea {
                        id: acceptMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            connectionDialog.accepted(device_ip, device_name)
                            if (connection_model) connection_model.accepted(device_ip, device_name)
                            connectionDialog.close()
                        }
                    }
                }
            }
        }
    }
}