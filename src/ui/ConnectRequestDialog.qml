// ConnectionRequestDialog.qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: connectionDialog
    width: 400
    height: 250
    title: "连接请求"
    modality: Qt.ApplicationModal
    flags: Qt.Dialog
    
    // 属性接口，用于外部设置设备信息
    property string device_ip: ""
    property string device_name: ""
    property var connection_model: null
    
    // 居中显示
    x: (Screen.width - width) / 2
    y: (Screen.height - height) / 2
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15
        
        // 标题
        Label {
            text: "连接请求"
            font.bold: true
            font.pixelSize: 18
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 10
        }
        
        // 设备信息区域
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "transparent"
            
            ColumnLayout {
                anchors.fill: parent
                spacing: 10
                
                // IP地址
                RowLayout {
                    Layout.fillWidth: true
                    Label {
                        text: "设备IP:"
                        font.bold: true
                        Layout.preferredWidth: 80
                    }
                    Label {
                        text: device_ip
                        Layout.fillWidth: true
                        wrapMode: Text.Wrap
                    }
                }
                
                // 设备名称
                RowLayout {
                    Layout.fillWidth: true
                    Label {
                        text: "设备名称:"
                        font.bold: true
                        Layout.preferredWidth: 80
                    }
                    Label {
                        text: device_name
                        Layout.fillWidth: true
                        wrapMode: Text.Wrap
                    }
                }
            }
        }
        
        // 按钮区域
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 20
            
            Button {
                id: rejectBtn
                text: "拒绝"
                Layout.preferredWidth: 100
                onClicked: {
                    connection_model.rejected(device_ip,device_name)
                    connectionDialog.close()
                }
            }
            
            Button {
                id: acceptBtn
                text: "接受"
                Layout.preferredWidth: 100
                highlighted: true
                onClicked: {
                    connection_model.accepted(device_ip,device_name)
                    connectionDialog.close()
                }
            }
        }
    }
    
    // 快捷键支持
    Shortcut {
        sequence: "Esc"
        onActivated: {
            connection_model.rejected(device_ip,device_name)
            connectionDialog.close()
        }
    }
}