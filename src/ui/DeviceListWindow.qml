import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls

Window {
    id: deviceListWindow
    width: 420
    height: 520
    color: "transparent"
    visible: false
    flags: Qt.FramelessWindowHint
    
    property var deviceModel: null
    
    LoadingDialog {
        id: load_dialog
        onButtonClicked: {
            deviceModel.resetConnection()
            load_dialog.hide()
        }
    }

    Loader {
        id: generalDialogLoader
        source: "qrc:/qml/ui/GeneralDialog.qml"
    }
    
    // 居中显示
    function centerOnScreen() {
        Qt.callLater(function() {
            var screenWidth = Screen.width > 0 ? Screen.width : 1920
            var screenHeight = Screen.height > 0 ? Screen.height : 1080
            
            deviceListWindow.x = Math.max(0, (screenWidth - deviceListWindow.width) / 2)
            deviceListWindow.y = Math.max(0, (screenHeight - deviceListWindow.height) / 2)
        })
    }
    
    onVisibleChanged: {
        if (visible) {
            centerOnScreen()
            requestActivate()
            if (deviceModel && deviceListView.count === 0) {
                deviceModel.startScan()
            }
        }
    }
    
    function showWindow(model) {
        deviceModel = model
        centerOnScreen()
        show()
        raise()
        requestActivate()
    }

    function closeLoadingDialog(){
        load_dialog.close()
    }
    
    // 处理扫描完成逻辑
    function handleScanComplete() {
        if (!deviceListWindow.visible) {
            // 窗口被隐藏了，显示窗口并弹出对话框
            deviceListWindow.show()
            deviceListWindow.raise()
            deviceListWindow.requestActivate()
            
            // 延迟一点确保窗口先显示
            Qt.callLater(function() {
                if (generalDialogLoader.status === Loader.Ready) {
                    var deviceCount = deviceModel ? deviceListView.count : 0
                    var message = deviceCount > 0 ? 
                        `扫描完成，发现 ${deviceCount} 个设备` : 
                        "扫描完成，未发现设备"
                    
                    var iconType = deviceCount > 0 ? 
                        generalDialogLoader.item.success : 
                        generalDialogLoader.item.info
                    
                    generalDialogLoader.item.showDialog("扫描完成", message, iconType, generalDialogLoader.item.ok)
                }
            })
        }
    }

    Connections {
        target: device_list_model
        enabled: deviceWindowLoader.status === Loader.Ready
                
        function onConnectResult(ret, ip) {
            deviceListWindow.hide()
            load_dialog.close()
        }
    }
    // 窗口主体
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
                        deviceListWindow.x += delta.x
                        deviceListWindow.y += delta.y
                    }
                }
                    
                // 双击最大化/还原
                onDoubleClicked:{
                    if (deviceListWindow.visibility === Window.Windowed) {
                        deviceListWindow.showMaximized()
                    } else {
                        deviceListWindow.showNormal()
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
                        text: "📱"
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
                    width: parent.width - titleIcon.width - scanButton.width - minimizeButton.width - closeButton.width - 12 * 5
                    height: parent.height
                    spacing: 2
                    
                    Text {
                        text: "设备列表"
                        font.pixelSize: 20
                        font.bold: true
                        font.family: "Microsoft YaHei UI"
                        color: "#1f2937"
                    }
                    
                    Text {
                        id: subtitleText
                        text: "局域网设备发现"
                        font.pixelSize: 13
                        font.family: "Microsoft YaHei UI"
                        color: "#9ca3af"
                    }
                }
                
                // 扫描/停止按钮
                Rectangle {
                    id: scanButton
                    width: 80
                    height: 36
                    radius: 8
                    color: scanMouse.containsMouse ? (deviceModel && deviceModel.scanning ? "#fef2f2" : "#f0f9ff") : "#f8fafc"
                    border.color: scanMouse.containsMouse ? (deviceModel && deviceModel.scanning ? "#fca5a5" : "#7dd3fc") : "#e2e8f0"
                    border.width: 1.5
                    
                    Text {
                        anchors.centerIn: parent
                        text: deviceModel && deviceModel.scanning ? "停止" : "扫描"
                        font.pixelSize: 14
                        font.family: "Microsoft YaHei UI"
                        font.weight: Font.Medium
                        color: deviceModel && deviceModel.scanning ? "#dc2626" : "#0369a1"
                    }
                    
                    MouseArea {
                        id: scanMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (deviceModel) {
                                if (!deviceModel.scanning) {
                                    deviceModel.refresh()
                                } else {
                                    deviceModel.stopScan()
                                    // 停止扫描时，如果窗口被隐藏，也显示窗口
                                    if (!deviceListWindow.visible) {
                                        deviceListWindow.show()
                                        deviceListWindow.raise()
                                        deviceListWindow.requestActivate()
                                    }
                                }
                            }
                        }
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
                            deviceListWindow.hide()
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
                        onClicked: deviceListWindow.close()
                    }
                }
            }
            

            // 快速连接卡片
            Rectangle {
                id: quickConnectCard
                width: parent.width
                height: 80
                radius: 12
                color: "#f8fafc"
                border.color: "#e2e8f0"
                border.width: 1
                anchors.top: titleRow.bottom
                anchors.topMargin: 16
                anchors.horizontalCenter: parent.horizontalCenter

                // 标题
                Text {
                    id: quickConnectTitle
                    text: "快速连接"
                    font.pixelSize: 13
                    font.family: "Microsoft YaHei UI"
                    font.weight: Font.Medium
                    color: "#64748b"
                    anchors.left: parent.left
                    anchors.leftMargin: 16
                    anchors.top: parent.top
                    anchors.topMargin: 12
                }

                RowLayout {
                    anchors.left: parent.left
                    anchors.leftMargin: 16
                    anchors.right: parent.right
                    anchors.rightMargin: 16
                    anchors.top: quickConnectTitle.bottom
                    anchors.topMargin: 8
                    spacing: 8

                    // IP地址输入部分（四个文本框和三个点）
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 0 
                        
                        Repeater {
                            id: ipInputFieldsRepeater
                            model: 4
                            
                            RowLayout {
                                spacing: 4
                                Layout.alignment: Qt.AlignVCenter
                                
                                TextField {
                                    id: ipField
                                    property int index: model.index
                                    property bool isLastField: index === 3
                                    
                                    Layout.preferredWidth: 56
                                    Layout.preferredHeight: 42
                                    font.pixelSize: 16
                                    horizontalAlignment: TextInput.AlignHCenter
                                    verticalAlignment: TextInput.AlignVCenter
                                    maximumLength: 3
                                    inputMethodHints: Qt.ImhDigitsOnly
                                    selectByMouse: true

                                    property bool isValidSegment: {
                                        if (text === "") return true
                                        var num = parseInt(text, 10)
                                        return !isNaN(num) && num >= 0 && num <= 255
                                    }
                                                
                                    background: Rectangle {
                                        color: "transparent"
                                        border.color: "#cbd5e1"
                                        border.width: 0
                                        Rectangle {
                                            anchors.left: parent.left
                                            anchors.right: parent.right
                                            anchors.bottom: parent.bottom
                                            height: 2
                                            color: {
                                                if (!ipField.isValidSegment) {
                                                    return "#ef4444" // 红色，表示无效
                                                } else if (ipField.activeFocus) {
                                                    return "#3b82f6" // 蓝色，表示聚焦
                                                } else {
                                                    return "#cbd5e1" // 灰色，默认
                                                }
                                            }
                                        }
                                    }
                                    
                                    validator: IntValidator { 
                                        bottom: 0; 
                                        top: 255 
                                    }
                                    
                                    // 自动跳转
                                    onTextChanged: {
                                        if (text.length >= 3 && !isLastField) {
                                            delayJumpTimer.index = index
                                            delayJumpTimer.start()
                                        }
                                    }
                                    
                                    Timer {
                                        id: delayJumpTimer
                                        interval: 10
                                        property int index: 0
                                        onTriggered: {
                                            if (ipInputFieldsRepeater.itemAt(index + 1)) {
                                                let nextContainer = ipInputFieldsRepeater.itemAt(index + 1)
                                                if (nextContainer && nextContainer.children[0]) {
                                                    nextContainer.children[0].forceActiveFocus()
                                                    nextContainer.children[0].selectAll()
                                                }
                                            }
                                        }
                                    }
                                    
                                    // 处理键盘事件
                                    Keys.onPressed: function(event) {
                                        // Backspace 且内容为空时，跳到上一段
                                        if (event.key === Qt.Key_Backspace && text === "") {
                                            if (index > 0) {
                                                let prevContainer = ipInputFieldsRepeater.itemAt(index - 1)
                                                if (prevContainer && prevContainer.children[0]) {
                                                    prevContainer.children[0].forceActiveFocus()
                                                    prevContainer.children[0].selectAll()
                                                    event.accepted = true
                                                }
                                            }
                                        }
                                        // 点号或右方向键跳到下一个
                                        else if ((event.key === Qt.Key_Period || event.key === Qt.Key_Right) && !isLastField) {
                                            let nextContainer = ipInputFieldsRepeater.itemAt(index + 1)
                                            if (nextContainer && nextContainer.children[0]) {
                                                nextContainer.children[0].forceActiveFocus()
                                                nextContainer.children[0].selectAll()
                                                event.accepted = true
                                            }
                                        }
                                        // 左方向键跳到上一个
                                        else if (event.key === Qt.Key_Left && index > 0) {
                                            let prevContainer = ipInputFieldsRepeater.itemAt(index - 1)
                                            if (prevContainer && prevContainer.children[0]) {
                                                prevContainer.children[0].forceActiveFocus()
                                                prevContainer.children[0].selectAll()
                                                event.accepted = true
                                            }
                                        }
                                        // 输入点号时自动跳到下一个
                                        else if (event.text === "." && !isLastField) {
                                            let nextContainer = ipInputFieldsRepeater.itemAt(index + 1)
                                            if (nextContainer && nextContainer.children[0]) {
                                                nextContainer.children[0].forceActiveFocus()
                                                nextContainer.children[0].selectAll()
                                                event.accepted = true
                                            }
                                        }
                                    }
                                    
                                    onFocusChanged: {
                                        if (focus) {
                                            selectAll()
                                        }
                                    }
                                }
                                
                                // 点号分隔符（前三个后有）
                                Label {
                                    visible: index < 3
                                    text: "."
                                    font.pixelSize: 16
                                    color: "#64748b"
                                    Layout.alignment: Qt.AlignVCenter
                                    Layout.leftMargin: 4
                                }
                            }
                        }
                    }
                    
                    // 连接按钮
                    Rectangle {
                        id: connectButton
                        Layout.preferredWidth: 80
                        Layout.preferredHeight: 42
                        radius: 6
                        color: connectMouseArea.containsMouse ? "#dbeafe" : "#f1f5f9"
                        border.color: "#3b82f6"
                        border.width: 1.5
                        
                        Text {
                            anchors.centerIn: parent
                            text: "连接"
                            font.pixelSize: 14
                            font.family: "Microsoft YaHei UI"
                            font.weight: Font.Medium
                            color: "#1d4ed8"
                        }
                        
                        MouseArea {
                            id: connectMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                // 获取完整的IP地址
                                let ipParts = []
                                for (let i = 0; i < 4; i++) {
                                    let container = ipInputFieldsRepeater.itemAt(i)
                                    if (container && container.children[0]) {
                                        ipParts.push(container.children[0].text)
                                    }
                                }
                                let ip = ipParts.join(".")
                                
                                if(device_list_model.isLocalIp(ip))
                                {
                                    if (generalDialogLoader.status === Loader.Ready) {
                                        generalDialogLoader.item.iconType = generalDialogLoader.item.error
                                        generalDialogLoader.item.text = "该IP为本地地址"
                                        generalDialogLoader.item.buttons = generalDialogLoader.item.ok
                                        generalDialogLoader.item.show()
                                        generalDialogLoader.item.requestActivate()
                                    }
                                    return
                                }
                                function isValidIPv4(ip) {
                                    const regex = /^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$/
                                    const match = ip.match(regex)
                                    if (!match) return false
                                    
                                    for (let i = 1; i <= 4; i++) {
                                        const num = parseInt(match[i], 10)
                                        if (num < 0 || num > 255) return false
                                    }
                                    return true
                                }
                                
                                if (isValidIPv4(ip)) {
                                    deviceModel.connectToTarget(ip)
                                    load_dialog.show("正在连接...", "取消")
                                    deviceModel.stopScan()
                                } else {
                                    // 显示错误对话框
                                    if (generalDialogLoader.status === Loader.Ready) {
                                        generalDialogLoader.item.iconType = generalDialogLoader.item.error
                                        generalDialogLoader.item.text = "请输入有效的 IPv4 地址（如 192.168.1.100）"
                                        generalDialogLoader.item.buttons = generalDialogLoader.item.ok
                                        generalDialogLoader.item.show()
                                        generalDialogLoader.item.requestActivate()
                                    }
                                }
                            }
                        }
                    }
                }
            }
            // 设备列表区域
            Rectangle {
                id: deviceListArea
                width: parent.width
                height: parent.height - titleRow.height - quickConnectCard.height - statusRow.height - 40
                anchors.top: quickConnectCard.bottom
                anchors.topMargin: 16
                radius: 12
                color: "#f8fafc"
                border.color: "#e2e8f0"
                border.width: 1
                
                // 列表标题
                Row {
                    id: listHeader
                    width: parent.width - 32
                    height: 40
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: 12
                    
                    Text {
                        id: titleText
                        text: "可用设备"
                        font.pixelSize: 13
                        font.family: "Microsoft YaHei UI"
                        font.weight: Font.Medium
                        color: "#64748b"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    
                    Item { 
                        width: parent.width - availableDevicesText.width - titleText.width
                        height: 1
                    }
                    
                    Text {
                        id: availableDevicesText
                        text: deviceModel ? `${deviceListView.count} 个设备` : "0 个设备"
                        font.pixelSize: 12
                        font.family: "Microsoft YaHei UI"
                        color: "#94a3b8"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                // 设备列表
                ListView {
                    id: deviceListView
                    width: parent.width
                    height: parent.height - listHeader.height - listHeader.anchors.topMargin
                    anchors.top: listHeader.bottom
                    model: deviceModel ? deviceModel : null
                    clip: true
                    spacing: 1
                    boundsBehavior: Flickable.StopAtBounds
                    highlight: null
                    currentIndex: -1
                    
                    // 空状态提示
                    Text {
                        anchors.centerIn: parent
                        text: "未发现设备"
                        color: "#94a3b8"
                        font.pixelSize: 14
                        visible: deviceListView.count === 0 && (!deviceModel || !deviceModel.scanning)
                    }
                    

                    delegate: Rectangle {
                        id: deviceItem
                        width: deviceListView.width - 2
                        height: 72
                        color: index % 2 === 0 ? "#FFFFFF" : "#F8FAFC"
                        radius: 8
                        anchors.horizontalCenter: parent.horizontalCenter

                        // 定义属性并添加默认值
                        property string deviceName: model.deviceName || "未知设备"
                        property string deviceIp: model.deviceIP || "IP未知"
                        property string deviceType: model.deviceType || "未知"

                        RowLayout {
                            id: rowLayout
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 16

                            // 设备图标
                            Rectangle {
                                id: avatarRect
                                width: 48
                                height: 48
                                radius: 8
                                color: "#E0EAFF"
                                Layout.preferredWidth: 48
                                Layout.preferredHeight: 48

                                // 图标内部
                                Rectangle {
                                    width: 40
                                    height: 40
                                    radius: 20
                                    anchors.centerIn: parent
                                    color: deviceItem.containsMouse ? "#1976D2" : "#2196F3"
                                    
                                    Behavior on color {
                                        ColorAnimation { duration: 150 }
                                    }
                                    
                                    Text {
                                        anchors.centerIn: parent
                                        text: deviceItem.deviceName ? deviceItem.deviceName.charAt(0) : "?"
                                        color: "white"
                                        font.bold: true
                                        font.pixelSize: 16
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
                            }

                            // 文字区域
                            Column {
                                id: textArea
                                spacing: 4
                                Layout.fillWidth: true

                                Text {
                                    text: deviceItem.deviceName  // 使用属性而不是直接model访问
                                    font.pixelSize: 16
                                    font.bold: true
                                    color: "#1E293B"
                                    elide: Text.ElideRight
                                }

                                Text {
                                    text: deviceItem.deviceIp  // 使用属性而不是直接model访问
                                    font.pixelSize: 13
                                    color: "#64748B"
                                    elide: Text.ElideRight
                                }
                            }

                            // 类型标签
                            Rectangle {
                                id: typeTagRect
                                height: 26
                                radius: 6
                                Layout.preferredWidth: typeTagText.implicitWidth + 16
                                Layout.alignment: Qt.AlignVCenter
                                color: deviceItem.containsMouse ? "#F0F9FF" : "#F1F5F9"  // 修改颜色使其可见
                                border.color: deviceItem.containsMouse ? "#7DD3FC" : "#CBD5E1"
                                border.width: 1

                                Text {
                                    id: typeTagText
                                    text: deviceItem.deviceType  // 使用属性而不是直接model访问
                                    font.pixelSize: 12
                                    font.family: "Microsoft YaHei UI"
                                    anchors.centerIn: parent
                                    color: deviceItem.containsMouse ? "#0369A1" : "#475569"
                                }
                            }
                        }

                        // 添加鼠标交互
                        MouseArea {
                            id: deviceMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            
                            property bool containsMouse: false
                            property bool pressed: false
                            
                            onEntered: containsMouse = true
                            onExited: {
                                containsMouse = false
                                pressed = false
                            }
                            onPressed: pressed = true
                            onReleased: pressed = false
                            
                            onClicked: {
                                if (deviceModel) {
                                    deviceModel.connectToTarget(index)
                                    load_dialog.show("等待对方响应", "取消")
                                    deviceModel.stopScan()
                                }
                            }
                        }
                    }

                }
            }
            
            // 底部状态栏
            Item {
                id: statusRow
                width: parent.width
                height: 24
                anchors.bottom: parent.bottom
                
                // 进度条容器 - 固定在左侧
                Item {
                    id: progressBarContainer
                    width: parent.width * 0.2
                    height: 24
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    visible: deviceModel && deviceModel.scanning
                    
                    // 进度条背景
                    Rectangle {
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width
                        height: 6
                        radius: 3
                        color: "#e2e8f0"
                        
                        // 进度填充
                        Rectangle {
                            id: progressFill
                            width: parent.width * (scanProgress.currentProgress / 100)
                            height: parent.height
                            radius: 3
                            gradient: Gradient {
                                GradientStop { position: 0.0; color: "#6366f1" }
                                GradientStop { position: 1.0; color: "#8b5cf6" }
                            }
                            
                            Behavior on width {
                                NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
                            }
                        }
                    }
                }
                
                // 进度百分比 - 在进度条右边
                Text {
                    id: progressText
                    anchors.left: progressBarContainer.visible ? progressBarContainer.right : parent.left
                    anchors.leftMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    visible: deviceModel && deviceModel.scanning
                    text: `${scanProgress.currentProgress}%`
                    font.pixelSize: 12
                    font.family: "Microsoft YaHei UI"
                    font.weight: Font.Medium
                    color: "#6366f1"
                }
                
                // 状态文本 - 占中间空间
                Text {
                    id: statusText
                    anchors.left: progressText.visible ? progressText.right : parent.left
                    anchors.leftMargin: 12
                    anchors.right: statusIndicator.left
                    anchors.rightMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    text: {
                        if (!deviceModel) return "模型未加载"
                        if (deviceModel.scanning) return "正在扫描..."
                        if (deviceListView.count === 0) return "未发现设备"
                        return `发现 ${deviceListView.count} 个设备`
                    }
                    font.pixelSize: 13
                    font.family: "Microsoft YaHei UI"
                    color: "#64748b"
                    elide: Text.ElideRight
                }
                
                // 状态指示器容器（加载指示器或完成状态）
                Item {
                    id: statusIndicator
                    width: 24
                    height: 24
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    
                    // 加载指示器 - 扫描时显示
                    Item {
                        id: spinnerItem
                        anchors.fill: parent
                        visible: deviceModel && deviceModel.scanning
                        
                        // 旋转动画容器
                        Rectangle {
                            id: spinnerContainer
                            anchors.centerIn: parent
                            width: 24
                            height: 24
                            color: "transparent"
                            
                            // 旋转动画
                            Canvas {
                                id: spinnerCanvas
                                anchors.fill: parent
                                
                                property real rotationAngle: 0
                                
                                onPaint: {
                                    var ctx = getContext("2d")
                                    ctx.clearRect(0, 0, width, height)
                                    
                                    var centerX = width / 2
                                    var centerY = height / 2
                                    var radius = Math.min(width, height) / 2 - 3
                                    
                                    // 绘制旋转弧线
                                    ctx.beginPath()
                                    ctx.arc(centerX, centerY, radius, 
                                            rotationAngle * Math.PI / 180, 
                                            rotationAngle * Math.PI / 180 + Math.PI * 0.75)
                                    ctx.lineWidth = 2
                                    ctx.strokeStyle = "#6366f1"
                                    ctx.stroke()
                                }
                                
                                // 旋转动画
                                RotationAnimation on rotationAngle {
                                    from: 0
                                    to: 360
                                    duration: 1000
                                    loops: Animation.Infinite
                                    running: deviceModel && deviceModel.scanning
                                }
                                
                                // 当旋转角度改变时重绘
                                onRotationAngleChanged: requestPaint()
                            }
                        }
                    }
                    
                    // 完成状态 - 扫描完成时显示
                    Rectangle {
                        id: completeStatus
                        anchors.fill: parent
                        radius: 12
                        color: "#d1fae5"
                        visible: deviceModel && !deviceModel.scanning
                        
                        Text {
                            anchors.centerIn: parent
                            text: "✓"
                            font.pixelSize: 14
                            color: "#065f46"
                            font.bold: true
                        }
                    }
                }
            }
        }
    }
    
    // 进度管理
    Item {
        id: scanProgress
        property int currentProgress: 0
    }
    
    Connections {
        target: deviceModel
        enabled: deviceModel !== null
        
        function onScanProgress(percent) {
            scanProgress.currentProgress = Math.min(percent, 100)
        }
        
        function onScanFinished() {
            deviceListWindow.handleScanComplete()
        }
        
        function onScanningChanged() {
            if (deviceModel && !deviceModel.scanning) {
                // 扫描停止时检查是否完成
                deviceListWindow.handleScanComplete()
            }
        }
    }
}