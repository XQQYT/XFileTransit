import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Window {
    id: deviceListWindow
    width: 400
    height: 500
    x: (Screen.width - width) / 2
    y: (Screen.height - height) / 2
    title: qsTr("设备列表")
    modality: Qt.ApplicationModal
    flags: Qt.Dialog
    
    property var deviceModel: null
        
    Loader {
        id: generalDialogLoader
        source: "qrc:/qml/ui/GeneralDialog.qml"
    }
    
    Connections {
        target: deviceModel
        function onConnectResult(ret, ip) {
        if (ret) {
            // 连接成功：隐藏设备窗口，对话框作为独立窗口显示
            deviceListWindow.hide()
            generalDialogLoader.item.iconType = generalDialogLoader.item.success
            generalDialogLoader.item.text = "连接成功"
            generalDialogLoader.item.x = (Screen.width - generalDialogLoader.item.width) / 2
            generalDialogLoader.item.y = (Screen.height - generalDialogLoader.item.height) / 2
        } else {
            // 连接被拒绝：不隐藏设备窗口，对话框相对于设备窗口居中
            generalDialogLoader.item.iconType = generalDialogLoader.item.error
            generalDialogLoader.item.text = "连接被拒绝"
            generalDialogLoader.item.x = deviceListWindow.x + (deviceListWindow.width - generalDialogLoader.item.width) / 2
            generalDialogLoader.item.y = deviceListWindow.y + (deviceListWindow.height - generalDialogLoader.item.height) / 2
        }
        load_dialog.hide()
        generalDialogLoader.item.show()
    }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        
        // 标题栏
        RowLayout {
            Layout.fillWidth: true
            
            Label {
                text: qsTr("局域网设备")
                font.bold: true
                font.pixelSize: 16
                Layout.fillWidth: true
            }
            
            Button {
                text: qsTr("刷新")
                onClicked: {
                    if (deviceModel && !deviceModel.scanning) {
                        deviceModel.refresh()
                    }
                }
            }
        }
        LoadingDialog{
            id: load_dialog
            onButtonClicked:{
                console.log("取消操作")
                load_dialog.hide()
            }
        }
        // 设备列表
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            
            ListView {
                id: deviceListView
                model: deviceModel ? deviceModel : null
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                highlight: null
                currentIndex: -1
                delegate: Rectangle {
                    id: deviceItem
                    width: deviceListView.width
                    height: 60
                    color: {
                        if (deviceItem.pressed) 
                            return "#bbdefb"  // 按下时的颜色
                        else if (deviceItem.containsMouse) 
                            return "#f5f5f5"  // 悬浮时的颜色
                        else 
                            return "transparent" // 默认透明
                    }
                    
                    // 添加圆角效果
                    radius: 4
                    
                    // 属性定义
                    property string deviceName: model.deviceName
                    property string deviceIp: model.deviceIP
                    property string deviceType: model.deviceType
                    property bool containsMouse: false
                    property bool pressed: false
                    
                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        
                        onEntered: deviceItem.containsMouse = true
                        onExited: {
                            deviceItem.containsMouse = false
                            deviceItem.pressed = false
                        }
                        onPressed: deviceItem.pressed = true
                        onReleased: deviceItem.pressed = false
                        onCanceled: deviceItem.pressed = false
                        
                        onClicked: {
                            deviceModel.stopScan()
                            deviceModel.connectToTarget(index)
                            load_dialog.show("等待对方响应","取消")
                        }
                        onDoubleClicked: {
                            console.log("双击设备:", deviceName, deviceIp)
                        }
                    }
                    
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 12
                        
                        // 设备图标 - 添加悬浮效果
                        Rectangle {
                            width: 40
                            height: 40
                            radius: 20
                            color: deviceItem.containsMouse ? "#1976D2" : "#2196F3"
                            
                            Behavior on color {
                                ColorAnimation { duration: 150 }
                            }
                            
                            Label {
                                anchors.centerIn: parent
                                text: deviceItem.deviceName.charAt(0)
                                color: "white"
                                font.bold: true
                            }
                            
                            // 在线状态指示器
                            Rectangle {
                                width: 12
                                height: 12
                                radius: 6
                                color: "#4CAF50"
                                border.width: 2
                                border.color: "white"
                                anchors.right: parent.right
                                anchors.bottom: parent.bottom
                            }
                        }
                        
                        // 设备信息
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4
                            
                            Label {
                                text: deviceItem.deviceName
                                font.bold: true
                                font.pixelSize: 14
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                                color: deviceItem.pressed ? "#1565C0" : (deviceItem.containsMouse ? "#1976D2" : "black")
                            }
                            
                            Label {
                                text: deviceItem.deviceIp
                                color: deviceItem.pressed ? "#444" : "#666"
                                font.pixelSize: 12
                                Layout.fillWidth: true
                            }
                        }
                        
                        // 设备类型标签
                        Label {
                            text: deviceType
                            color: deviceItem.containsMouse ? "#666" : "#999"
                            font.pixelSize: 11
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            Layout.fillWidth: true
                        }
                    }
                    
                    // 底部边框 - 只在非悬浮状态下显示
                    Rectangle {
                        anchors.bottom: parent.bottom
                        width: parent.width
                        height: deviceItem.containsMouse ? 0 : 1  // 悬浮时隐藏边框
                        color: "#eeeeee"
                        Behavior on height {
                            NumberAnimation { duration: 150 }
                        }
                    }
                }
                
                // 空状态提示
                Label {
                    anchors.centerIn: parent
                    text: qsTr("未发现设备")
                    color: "#999"
                    visible: deviceListView.count === 0
                }
            }
        }
        
        // 状态栏
        RowLayout {
            Layout.fillWidth: true
            
            Label {
                id: statusLabel
                text: {
                    if (!deviceModel) return qsTr("模型未加载")
                    if (deviceListView.count === 0) return qsTr("未发现设备")
                    return qsTr("发现 %1 个设备").arg(deviceListView.count)
                }
                color: "#666"
                Layout.fillWidth: true
            }
            
            Item {
                Layout.preferredWidth: 80  // 给右侧区域固定宽度
                Layout.alignment: Qt.AlignRight
                
                BusyIndicator {
                    id: refreshIndicator
                    running: deviceModel ? deviceModel.scanning : false
                    anchors.centerIn: parent
                    width: 35
                    height: 35
                }
                
                Text {
                    text: "搜索完毕"
                    visible: !refreshIndicator.running && deviceModel && !deviceModel.scanning
                    anchors.centerIn: parent
                }
            }
        }
    }
    onVisibleChanged: {
        if (visible && deviceModel) {
            if (deviceListView.count === 0) {
                deviceModel.startScan()
            }
        }
    }
}