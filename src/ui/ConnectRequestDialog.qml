import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15

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
        radius: 14
        color: "#ffffff"
        border.color: "#f0f0f0"
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
                color: "#1f2937"
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
                        text: qsTr("IP地址")
                        font.pixelSize: 13
                        color: "#6b7280"
                        font.weight: Font.Medium
                    }
                    
                    Rectangle {
                        Layout.fillWidth: true
                        height: 40
                        radius: 8
                        color: "#f8fafc"
                        border.color: "#e2e8f0"
                        border.width: 1
                        
                        Text {
                            anchors.centerIn: parent
                            text: device_ip
                            font.pixelSize: 15
                            color: "#1e293b"
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
                        color: "#6b7280"
                        font.weight: Font.Medium
                    }
                    
                    Rectangle {
                        Layout.fillWidth: true
                        height: 40
                        radius: 8
                        color: "#f8fafc"
                        border.color: "#e2e8f0"
                        border.width: 1
                        
                        Text {
                            anchors.centerIn: parent
                            text: device_name
                            font.pixelSize: 15
                            color: "#1e293b"
                            font.bold: true
                        }
                    }
                }
            }
            
            // 提示文字
            Text {
                text: qsTr("是否允许此设备连接到您的计算机？")
                font.pixelSize: 13
                color: "#9ca3af"
                Layout.alignment: Qt.AlignHCenter
            }
            
            // 按钮区域（正确布局）
            Row {
                spacing: 12
                Layout.alignment: Qt.AlignHCenter
                
                // 拒绝按钮
                Rectangle {
                    width: 90
                    height: 38
                    radius: 8
                    color: rejectMouse.containsMouse ? "#fef2f2" : "#fafafa"
                    border.color: rejectMouse.containsMouse ? "#fca5a5" : "#e5e7eb"
                    border.width: 1.5
                    
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("拒绝")
                        font.pixelSize: 14
                        color: rejectMouse.containsMouse ? "#dc2626" : "#6b7280"
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
                    radius: 8
                    color: acceptMouse.containsMouse ? "#4f46e5" : "#6366f1"
                    
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("接受")
                        font.pixelSize: 14
                        color: "white"
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