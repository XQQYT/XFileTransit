import QtQuick 2.15
import QtQuick.Layouts 1.15

Window {
    id: ipInfoDialog
    title: qsTr("网络信息")
    width: 400
    height: 350
    minimumWidth: 300
    minimumHeight: 250
    flags: Qt.Dialog | Qt.FramelessWindowHint
    modality: Qt.ApplicationModal
    color: "transparent"
    visible: false
    
    property color primaryColor: "#6366F1"
    property color bgColor: "#FFFFFF"
    property color cardColor: "#F8FAFC"
    property color borderColor: "#E2E8F0"
    property color textPrimary: "#1E293B"
    property color textSecondary: "#64748B"
    
    property var networkInfoModel: null
    
    // 居中显示
    function centerOnScreen() {
        var screenWidth = Screen.width > 0 ? Screen.width : 1920
        var screenHeight = Screen.height > 0 ? Screen.height : 1080
            
        ipInfoDialog.x = Math.max(0, (screenWidth - ipInfoDialog.width) / 2)
        ipInfoDialog.y = Math.max(0, (screenHeight - ipInfoDialog.height) / 2)
    }
    
    onVisibleChanged: {
        copyBtn.text = qsTr("复制全部信息")
        if (visible) {
            centerOnScreen()
            requestActivate()
        }
    }
    
    function show() {
        ipInfoDialog.visible = true
        ipInfoDialog.raise()
        ipInfoDialog.requestActivate()
    }
    
    Rectangle {
        anchors.fill: parent
        radius: 16
        color: "#ffffff"
        
        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: "transparent"
            border.color: "#f0f0f0"
            border.width: 1
            
            // 鼠标区域用于拖动
            MouseArea {
                id: windowDragArea
                anchors.fill: parent
                property point clickPos: "0,0"
                    
                onPressed: function(mouse) {
                    clickPos = Qt.point(mouse.x, mouse.y)
                }
                    
                onPositionChanged: function(mouse) {
                    if (pressed) {
                        var delta = Qt.point(mouse.x - clickPos.x, mouse.y - clickPos.y)
                        ipInfoDialog.x += delta.x
                        ipInfoDialog.y += delta.y
                    }
                }
            }
        }
        
        // 主布局区域
        Item {
            id: mainContainer
            anchors.fill: parent
            anchors.margins: 20

            // 标题栏
            Row {
                id: titleRow
                width: parent.width
                height: 44
                spacing: 12

                // 标题图标
                Rectangle {
                    id: titleIcon
                    width: 44
                    height: 44
                    radius: 12
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#6366f1" }
                        GradientStop { position: 1.0; color: "#8b5cf6" }
                    }
                    
                    Text {
                        anchors.centerIn: parent
                        text: "🌐"
                        font.pixelSize: 22
                        font.bold: true
                    }
                    
                    // 光泽效果
                    Rectangle {
                        width: parent.width
                        height: parent.height * 0.3
                        radius: 6
                        color: "#40ffffff"
                        anchors.top: parent.top
                    }
                }
                
                Column {
                    id: titleTextColumn
                    width: parent.width - titleIcon.width - minimizeButton.width - closeButton.width - 12 * 3
                    height: parent.height
                    spacing: 2
                    
                    Text {
                        text: qsTr("网络信息")
                        font.pixelSize: 20
                        font.bold: true
                        font.family: "Microsoft YaHei UI"
                        color: "#1f2937"
                    }
                    
                    Text {
                        id: subtitleText
                        text: qsTr("IP地址和网段信息")
                        font.pixelSize: 13
                        font.family: "Microsoft YaHei UI"
                        color: "#9ca3af"
                    }
                }
                
                // 最小化按钮
                Rectangle {
                    id: minimizeButton
                    width: 28
                    height: 28
                    radius: 14
                    color: minimizeMouse.containsMouse ? "#f3f4f6" : "transparent"
                    
                    Text {
                        anchors.centerIn: parent
                        text: "−"
                        font.pixelSize: 20
                        color: minimizeMouse.containsMouse ? "#6b7280" : "#9ca3af"
                    }
                    
                    MouseArea {
                        id: minimizeMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            ipInfoDialog.hide()
                        }
                    }
                }
                
                // 关闭按钮
                Rectangle {
                    id: closeButton
                    width: 28
                    height: 28
                    radius: 14
                    color: closeMouse.containsMouse ? "#f3f4f6" : "transparent"
                    
                    Text {
                        anchors.centerIn: parent
                        text: "×"
                        font.pixelSize: 20
                        color: closeMouse.containsMouse ? "#6b7280" : "#9ca3af"
                    }
                    
                    MouseArea {
                        id: closeMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: ipInfoDialog.close()
                    }
                }
            }
            
            // 列表标题
            Row {
                id: listHeader
                width: parent.width
                height: 40
                anchors.top: titleRow.bottom
                anchors.topMargin: 8
                spacing: 5

                Text {
                    id: titleText
                    text: qsTr("网络信息列表")
                    font.pixelSize: 13
                    font.family: "Microsoft YaHei UI"
                    font.weight: Font.Medium
                    color: "#64748b"
                    anchors.verticalCenter: parent.verticalCenter
                }
                
                // 刷新按钮
                Rectangle {
                    id: refreshButton
                    width: 24
                    height: 24
                    radius: 6
                    color: refreshMouseArea.containsMouse ? "#f1f5f9" : "transparent"
                    anchors.verticalCenter: parent.verticalCenter
                    
                    Text {
                        anchors.centerIn: parent
                        text: "↻"
                        font.pixelSize: 20
                        color: refreshMouseArea.containsMouse ? "#3b82f6" : "#94a3b8"
                    }
                    
                    MouseArea {
                        id: refreshMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            refreshButton.visible = false
                            networkInfoModel.refreshNetInfo()
                            refreshButton.visible = true
                        }
                    }
                }
                Item { 
                    width: parent.width - listCountText.width - titleText.width - refreshButton.width
                    height: 1
                }
                
                Text {
                    id: listCountText
                    text: networkListView.count + qsTr(" 项")
                    font.pixelSize: 12
                    font.family: "Microsoft YaHei UI"
                    color: "#94a3b8"
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            // 网络信息列表区域
            Rectangle {
                id: listArea
                width: parent.width
                height: parent.height - titleRow.height - listHeader.height - actionButton.height - 32
                anchors.top: listHeader.bottom
                anchors.topMargin: 8
                radius: 12
                color: "#f8fafc"
                border.color: "#e2e8f0"
                border.width: 1
                
                // 表头列标题
                Rectangle {
                    id: columnHeaders
                    width: parent.width - 2
                    height: 40
                    color: "#f8fafc"
                    anchors.top: parent.top
                    anchors.topMargin: 1
                    anchors.left: parent.left
                    anchors.leftMargin: 1
                    z: 1
                    
                    Row {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 8
                        
                        // IP地址列标题
                        Text {
                            width: 150
                            text: qsTr("IP地址")
                            font.pixelSize: 12
                            font.bold: true
                            font.family: "Microsoft YaHei UI"
                            color: textPrimary
                        }
                        
                        // 分隔线
                        Rectangle {
                            width: 1
                            height: 20
                            color: borderColor
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        
                        // 网段列标题
                        Text {
                            width: parent.width - 150 - 8 - 1
                            text: qsTr("网段(CIDR)")
                            font.pixelSize: 12
                            font.bold: true
                            font.family: "Microsoft YaHei UI"
                            color: textPrimary
                        }
                    }
                }
                
                // 网络信息列表
                ListView {
                    id: networkListView
                    anchors.fill: parent
                    anchors.margins: 1
                    anchors.topMargin: 40
                    model: networkInfoModel
                    clip: true
                    spacing: 1
                    boundsBehavior: Flickable.StopAtBounds
                    highlight: null
                    currentIndex: -1
                    
                    // 空状态提示
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("暂无网络信息")
                        color: "#94a3b8"
                        font.pixelSize: 14
                        visible: networkListView.count === 0
                    }

                    // 列表内容
                    delegate: Rectangle {
                        id: networkItem
                        width: networkListView.width - 2
                        height: 60
                        color: index % 2 === 0 ? "#FFFFFF" : "#F8FAFC"
                        radius: 8
                        // anchors.horizontalCenter: parent.horizontalCenter

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 8
                            
                            // IP地址列
                            Column {
                                Layout.preferredWidth: 150
                                Layout.alignment: Qt.AlignVCenter
                                
                                Text {
                                    text: model.ip
                                    font.pixelSize: 14
                                    font.bold: true
                                    font.family: "Microsoft YaHei UI"
                                    color: textPrimary
                                    width: parent.width
                                    elide: Text.ElideRight
                                }
                            }
                            
                            // 分隔线
                            Rectangle {
                                width: 1
                                height: 40
                                color: borderColor
                                Layout.alignment: Qt.AlignVCenter
                            }
                            
                            // 网段列
                            Column {
                                Layout.fillWidth: true
                                Layout.alignment: Qt.AlignVCenter
                                
                                Text {
                                    text: model.cidr
                                    font.pixelSize: 14
                                    font.bold: true
                                    font.family: "Microsoft YaHei UI"
                                    color: "#065F46"
                                    width: parent.width
                                    elide: Text.ElideRight
                                }
                            }
                        }
                        
                        // 鼠标悬停效果
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            
                            property bool containsMouse: false
                            
                            onEntered: containsMouse = true
                            onExited: containsMouse = false
                            
                        }
                    }
                }
            }
            
            // 底部操作按钮
            Rectangle {
                id: actionButton
                width: 120
                height: 36
                radius: 18
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                color: copyAllMouse.containsMouse ? Qt.darker(primaryColor, 1.1) : primaryColor
                    
                Text {
                    id: copyBtn
                    text: qsTr("复制全部信息")
                    font.pixelSize: 13
                    font.bold: true
                    font.family: "Microsoft YaHei UI"
                    color: "white"
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                
                MouseArea {
                    id: copyAllMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        networkInfoModel.copyNetInfoText()
                        copyBtn.text = qsTr("已复制到剪切板")
                    }
                }
            }
        }
    }
}