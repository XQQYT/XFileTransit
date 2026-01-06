import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

Window {
    id: deviceListWindow
    width: 420
    height: 520
    color: "transparent"
    visible: false
    flags: Qt.FramelessWindowHint
    
    property var deviceModel: null
    property int connectionMode: 0
    
    property color windowBg: "#ffffff"
    property color windowBorder: "#f0f0f0"
    property int windowRadius: 16
    
    property color textPrimary: "#1f2937"
    property color textSecondary: "#6b7280"
    property color textTertiary: "#9ca3af"
    property color textDisabled: "#94a3b8"
    property color textOnPrimary: "white"
    property color textOnAccent: "#1d4ed8"
    property color textError: "#dc2626"
    property color textSuccess: "#065f46"
    property color textInfo: "#0369a1"
    property color textAccent: "#6366f1"
    
    property color cardBg: "#f8fafc"
    property color cardBorder: "#e2e8f0"
    property int cardRadius: 12
    
    property color buttonBg: "#f1f5f9"
    property color buttonBgHover: "#dbeafe"
    property color buttonBorder: "#3b82f6"
    property color buttonText: "#1d4ed8"
    
    property color scanButtonBg: "#f8fafc"
    property color scanButtonBgHover: "#f0f9ff"
    property color scanButtonBgActive: "#fef2f2"
    property color scanButtonBorder: "#e2e8f0"
    property color scanButtonBorderHover: "#7dd3fc"
    property color scanButtonBorderActive: "#fca5a5"
    property color scanButtonText: "#0369a1"
    property color scanButtonTextActive: "#dc2626"
    
    property color closeButtonBg: "transparent"
    property color closeButtonBgHover: "#f3f4f6"
    property color closeButtonText: "#9ca3af"
    property color closeButtonTextHover: "#6b7280"
    
    property color titleIconGradientStart: "#6366f1"
    property color titleIconGradientEnd: "#8b5cf6"
    property color titleIconGlow: "#40ffffff"
    
    property color inputFieldBg: "transparent"
    property color inputFieldBorderDefault: "#cbd5e1"
    property color inputFieldBorderFocus: "#3b82f6"
    property color inputFieldBorderError: "#ef4444"
    property color inputFieldUnderline: "#cbd5e1"
    
    property color deviceItemBg1: "#FFFFFF"
    property color deviceItemBg2: "#F8FAFC"
    property color deviceIconBg: "#E0EAFF"
    property color deviceIconInner: "#2196F3"
    property color deviceIconInnerHover: "#1976D2"
    property color deviceStatusOnline: "#4CAF50"
    property color deviceStatusBorder: "white"
    
    property color tagBg: "#F1F5F9"
    property color tagBgHover: "#F0F9FF"
    property color tagBorder: "#CBD5E1"
    property color tagBorderHover: "#7DD3FC"
    property color tagText: "#475569"
    property color tagTextHover: "#0369A1"
    
    property color progressBarBg: "#e2e8f0"
    property color progressBarGradientStart: "#6366f1"
    property color progressBarGradientEnd: "#8b5cf6"
    
    property color spinnerColor: "#6366f1"
    property color completeStatusBg: "#d1fae5"
    property color completeStatusText: "#065f46"
    
    property color listHeaderBg: "#f8fafc"
    property color listHeaderText: "#64748b"
    
    // 主题切换函数
    function setTheme(theme_index) {
        load_dialog.setTheme(theme_index)
        switch(theme_index)
        {
            case 0:
                //浅色主题
                windowBg = "#ffffff"
                windowBorder = "#f0f0f0"
                
                textPrimary = "#1f2937"
                textSecondary = "#6b7280"
                textTertiary = "#9ca3af"
                textDisabled = "#94a3b8"
                textOnPrimary = "white"
                textOnAccent = "#1d4ed8"
                textError = "#dc2626"
                textSuccess = "#065f46"
                textInfo = "#0369a1"
                textAccent = "#6366f1"
                
                cardBg = "#f8fafc"
                cardBorder = "#e2e8f0"
                
                buttonBg = "#f1f5f9"
                buttonBgHover = "#dbeafe"
                buttonBorder = "#3b82f6"
                buttonText = "#1d4ed8"
                
                scanButtonBg = "#f8fafc"
                scanButtonBgHover = "#f0f9ff"
                scanButtonBgActive = "#fef2f2"
                scanButtonBorder = "#e2e8f0"
                scanButtonBorderHover = "#7dd3fc"
                scanButtonBorderActive = "#fca5a5"
                scanButtonText = "#0369a1"
                scanButtonTextActive = "#dc2626"
                
                closeButtonBg = "transparent"
                closeButtonBgHover = "#f3f4f6"
                closeButtonText = "#9ca3af"
                closeButtonTextHover = "#6b7280"
                
                titleIconGradientStart = "#6366f1"
                titleIconGradientEnd = "#8b5cf6"
                titleIconGlow = "#40ffffff"
                
                inputFieldBg = "transparent"
                inputFieldBorderDefault = "#cbd5e1"
                inputFieldBorderFocus = "#3b82f6"
                inputFieldBorderError = "#ef4444"
                inputFieldUnderline = "#cbd5e1"
                
                deviceItemBg1 = "#FFFFFF"
                deviceItemBg2 = "#F8FAFC"
                deviceIconBg = "#E0EAFF"
                deviceIconInner = "#2196F3"
                deviceIconInnerHover = "#1976D2"
                deviceStatusOnline = "#4CAF50"
                deviceStatusBorder = "white"
                
                tagBg = "#F1F5F9"
                tagBgHover = "#F0F9FF"
                tagBorder = "#CBD5E1"
                tagBorderHover = "#7DD3FC"
                tagText = "#475569"
                tagTextHover = "#0369A1"
                
                progressBarBg = "#e2e8f0"
                progressBarGradientStart = "#6366f1"
                progressBarGradientEnd = "#8b5cf6"
                
                spinnerColor = "#6366f1"
                completeStatusBg = "#d1fae5"
                completeStatusText = "#065f46"
                
                listHeaderBg = "#f8fafc"
                listHeaderText = "#64748b"
                break
            case 1:
                //深色主题
                windowBg = "#1f2937"
                windowBorder = "#374151"
                
                textPrimary = "#f9fafb"
                textSecondary = "#d1d5db"
                textTertiary = "#9ca3af"
                textDisabled = "#6b7280"
                textOnPrimary = "white"
                textOnAccent = "#93c5fd"
                textError = "#fca5a5"
                textSuccess = "#86efac"
                textInfo = "#7dd3fc"
                textAccent = "#6366f1"
                
                cardBg = "#111827"
                cardBorder = "#374151"
                
                buttonBg = "#374151"
                buttonBgHover = "#1e40af"
                buttonBorder = "#3b82f6"
                buttonText = "#93c5fd"
                
                scanButtonBg = "#374151"
                scanButtonBgHover = "#0c4a6e"
                scanButtonBgActive = "#7f1d1d"
                scanButtonBorder = "#4b5563"
                scanButtonBorderHover = "#0369a1"
                scanButtonBorderActive = "#991b1b"
                scanButtonText = "#7dd3fc"
                scanButtonTextActive = "#fca5a5"
                
                closeButtonBg = "transparent"
                closeButtonBgHover = "#4b5563"
                closeButtonText = "#9ca3af"
                closeButtonTextHover = "#d1d5db"
                
                titleIconGradientStart = "#6366f1"
                titleIconGradientEnd = "#8b5cf6"
                titleIconGlow = "#20ffffff"
                
                inputFieldBg = "transparent"
                inputFieldBorderDefault = "#4b5563"
                inputFieldBorderFocus = "#3b82f6"
                inputFieldBorderError = "#ef4444"
                inputFieldUnderline = "#4b5563"
                
                deviceItemBg1 = "#111827"
                deviceItemBg2 = "#1f2937"
                deviceIconBg = "#1e40af"
                deviceIconInner = "#3b82f6"
                deviceIconInnerHover = "#1d4ed8"
                deviceStatusOnline = "#10b981"
                deviceStatusBorder = "#1f2937"
                
                tagBg = "#374151"
                tagBgHover = "#0c4a6e"
                tagBorder = "#4b5563"
                tagBorderHover = "#0369a1"
                tagText = "#d1d5db"
                tagTextHover = "#7dd3fc"
                
                progressBarBg = "#374151"
                progressBarGradientStart = "#6366f1"
                progressBarGradientEnd = "#8b5cf6"
                
                spinnerColor = "#6366f1"
                completeStatusBg = "#064e3b"
                completeStatusText = "#86efac"
                
                listHeaderBg = "#111827"
                listHeaderText = "#9ca3af"
                break
            default:
                return
        }
    }
    
    LoadingDialog {
        id: load_dialog
        onButtonClicked: {
            deviceModel.resetConnection()
            load_dialog.hide()
        }
    }

    Loader {
        id: generalDialogLoader
        source: "qrc:/ui/GeneralDialog.qml"
        onLoaded:{
            item.transientParent = deviceListWindow
        }
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
                        qsTr("扫描完成，发现 %1 个设备").arg(deviceCount) : 
                        qsTr("扫描完成，未发现设备")
                    
                    var iconType = deviceCount > 0 ? 
                        generalDialogLoader.item.success : 
                        generalDialogLoader.item.info
                    
                    generalDialogLoader.item.showDialog(qsTr("扫描完成"), message, iconType, generalDialogLoader.item.ok)
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
        radius: windowRadius
        color: windowBg
        
        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: "transparent"
            border.color: windowBorder
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
                        GradientStop { position: 0.0; color: titleIconGradientStart }
                        GradientStop { position: 1.0; color: titleIconGradientEnd }
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
                        color: titleIconGlow
                        anchors.top: parent.top
                    }
                }
                
                Column {
                    id: titleTextColumn
                    width: parent.width - titleIcon.width - scanButton.width - minimizeButton.width - closeButton.width - 12 * 5
                    height: parent.height
                    spacing: 2
                    
                    Text {
                        text: qsTr("设备列表")
                        font.pixelSize: 20
                        font.bold: true
                        font.family: "Microsoft YaHei UI"
                        color: textPrimary
                    }
                    
                    Text {
                        id: subtitleText
                        text: qsTr("局域网设备发现")
                        font.pixelSize: 13
                        font.family: "Microsoft YaHei UI"
                        color: textTertiary
                    }
                }
                
                // 扫描/停止按钮
                Rectangle {
                    id: scanButton
                    width: 80
                    height: 36
                    radius: 8
                    color: scanMouse.containsMouse ? (deviceModel && deviceModel.scanning ? scanButtonBgActive : scanButtonBgHover) : scanButtonBg
                    border.color: scanMouse.containsMouse ? (deviceModel && deviceModel.scanning ? scanButtonBorderActive : scanButtonBorderHover) : scanButtonBorder
                    border.width: 1.5
                    
                    Text {
                        anchors.centerIn: parent
                        text: deviceModel && deviceModel.scanning ? qsTr("停止") : qsTr("扫描")
                        font.pixelSize: 14
                        font.family: "Microsoft YaHei UI"
                        font.weight: Font.Medium
                        color: deviceModel && deviceModel.scanning ? scanButtonTextActive : scanButtonText
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
                    color: minimizeMouse.containsMouse ? closeButtonBgHover : closeButtonBg
                    
                    Text {
                        anchors.centerIn: parent
                        text: "−"
                        font.pixelSize: 20
                        color: minimizeMouse.containsMouse ? closeButtonTextHover : closeButtonText
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
                    color: closeMouse.containsMouse ? closeButtonBgHover : closeButtonBg
                    
                    Text {
                        anchors.centerIn: parent
                        text: "×"
                        font.pixelSize: 20
                        color: closeMouse.containsMouse ? closeButtonTextHover : closeButtonText
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
                height: connectionMode === 1 ? 145 : 120
                radius: cardRadius
                color: cardBg
                border.color: cardBorder
                border.width: 1
                anchors.top: titleRow.bottom
                anchors.topMargin: 16
                anchors.horizontalCenter: parent.horizontalCenter

                // 模式切换按钮
                Row {
                    id: modeSwitch
                    anchors.top: parent.top
                    anchors.topMargin: 12
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 0
                    height: 28
                    
                    // 局域网模式按钮
                    Rectangle {
                        id: lanModeButton
                        width: 80
                        height: 28
                        radius: 6
                        color: connectionMode === 0 ? buttonBg : "transparent"
                        border.color: connectionMode === 0 ? buttonBorder : textDisabled
                        border.width: 1
                        
                        Text {
                            anchors.centerIn: parent
                            text: qsTr("局域网")
                            font.pixelSize: 12
                            font.family: "Microsoft YaHei UI"
                            font.weight: connectionMode === 0 ? Font.Medium : Font.Normal
                            color: connectionMode === 0 ? buttonText : textSecondary
                        }
                        
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (connectionMode !== 0) {
                                    connectionMode = 0
                                }
                            }
                            
                            onEntered: {
                                if (connectionMode !== 0) {
                                    lanModeButton.color = closeButtonBgHover
                                }
                            }
                            onExited: {
                                if (connectionMode !== 0) {
                                    lanModeButton.color = "transparent"
                                }
                            }
                        }
                    }
                    
                    // P2P模式按钮
                    Rectangle {
                        id: p2pModeButton
                        width: 80
                        height: 28
                        radius: 6
                        color: connectionMode === 1 ? buttonBg : "transparent"
                        border.color: connectionMode === 1 ? buttonBorder : textDisabled
                        border.width: 1
                        
                        Text {
                            anchors.centerIn: parent
                            text: qsTr("P2P")
                            font.pixelSize: 12
                            font.family: "Microsoft YaHei UI"
                            font.weight: connectionMode === 1 ? Font.Medium : Font.Normal
                            color: connectionMode === 1 ? buttonText : textSecondary
                        }
                        
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (connectionMode !== 1) {
                                    connectionMode = 1
                                }
                            }
                            
                            onEntered: {
                                if (connectionMode !== 1) {
                                    p2pModeButton.color = closeButtonBgHover
                                }
                            }
                            onExited: {
                                if (connectionMode !== 1) {
                                    p2pModeButton.color = "transparent"
                                }
                            }
                        }
                    }
                }
                    
                // 标题
                Text {
                    id: quickConnectTitle
                    text: qsTr("快速连接")
                    font.pixelSize: 13
                    font.family: "Microsoft YaHei UI"
                    font.weight: Font.Medium
                    color: listHeaderText
                    anchors.left: parent.left
                    anchors.leftMargin: 16
                    anchors.top: modeSwitch.bottom
                    anchors.topMargin: 8
                    visible: connectionMode === 0
                }

                // 局域网模式内容
                Rectangle {
                    id: lanModeContent
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: quickConnectTitle.bottom
                    anchors.topMargin: 8
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 12
                    color: "transparent"
                    visible: connectionMode === 0
                    
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 16
                        anchors.rightMargin: 16
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
                                            color: inputFieldBg
                                            border.color: inputFieldBorderDefault
                                            border.width: 0
                                            Rectangle {
                                                anchors.left: parent.left
                                                anchors.right: parent.right
                                                anchors.bottom: parent.bottom
                                                height: 2
                                                color: {
                                                    if (!ipField.isValidSegment) {
                                                        return textError
                                                    } else if (ipField.activeFocus) {
                                                        return inputFieldBorderFocus
                                                    } else {
                                                        return inputFieldUnderline
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
                                        color: textDisabled
                                        Layout.alignment: Qt.AlignVCenter
                                        Layout.leftMargin: 4
                                    }
                                }
                            }
                        }
                        
                        // 连接按钮
                        Rectangle {
                            id: lanConnectButton
                            Layout.preferredWidth: 80
                            Layout.preferredHeight: 42
                            radius: 6
                            color: lanConnectMouseArea.containsMouse ? buttonBgHover : buttonBg
                            border.color: buttonBorder
                            border.width: 1.5
                            
                            Text {
                                anchors.centerIn: parent
                                text: qsTr("连接")
                                font.pixelSize: 14
                                font.family: "Microsoft YaHei UI"
                                font.weight: Font.Medium
                                color: buttonText
                            }
                            
                            MouseArea {
                                id: lanConnectMouseArea
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
                                            generalDialogLoader.item.text = qsTr("该IP为本地地址")
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
                                        load_dialog.show(qsTr("正在连接..."), qsTr("取消"))
                                        deviceModel.stopScan()
                                    } else {
                                        // 显示错误对话框
                                        if (generalDialogLoader.status === Loader.Ready) {
                                            generalDialogLoader.item.iconType = generalDialogLoader.item.error
                                            generalDialogLoader.item.text = qsTr("请输入有效的 IPv4 地址（如 192.168.1.100)")
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
                
                // P2P模式内容
                Rectangle {
                    id: p2pModeContent
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: modeSwitch.bottom
                    anchors.topMargin: 8
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 12
                    color: "transparent"
                    visible: connectionMode === 1
                    
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 16
                        anchors.rightMargin: 16
                        spacing: 8
                        
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 8
                            
                            // 对方代码输入
                            RowLayout {
                                spacing: 8
                                Layout.fillWidth: true
                                
                                Text {
                                    text: qsTr("对方代码:")
                                    font.pixelSize: 14
                                    font.family: "Microsoft YaHei UI"
                                    color: listHeaderText
                                    Layout.preferredWidth: 70
                                }
                                
                                TextField {
                                    id: p2pCodeField
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 42
                                    font.pixelSize: 12
                                    placeholderText: qsTr("请输入对方设备代码")
                                    placeholderTextColor: textTertiary
                                    maximumLength: 12
                                    inputMethodHints: Qt.ImhUppercaseOnly | Qt.ImhNoPredictiveText
                                    selectByMouse: true
                                    
                                    background: Rectangle {
                                        color: inputFieldBg
                                        border.color: inputFieldBorderDefault
                                        border.width: 0
                                        Rectangle {
                                            anchors.left: parent.left
                                            anchors.right: parent.right
                                            anchors.bottom: parent.bottom
                                            height: 2
                                            color: {
                                                if (p2pCodeField.activeFocus) {
                                                    return inputFieldBorderFocus
                                                } else {
                                                    return inputFieldUnderline
                                                }
                                            }
                                        }
                                    }
                                    
                                    onFocusChanged: {
                                        if (focus) {
                                            selectAll()
                                        }
                                    }
                                }
                            }
                            
                            // 动态密码输入
                            RowLayout {
                                spacing: 8
                                Layout.fillWidth: true
                                
                                Text {
                                    text: qsTr("动态密码:")
                                    font.pixelSize: 14
                                    font.family: "Microsoft YaHei UI"
                                    color: listHeaderText
                                    Layout.preferredWidth: 70
                                }
                                
                                TextField {
                                    id: p2pPasswordField
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 42
                                    font.pixelSize: 12
                                    placeholderText: qsTr("请输入动态密码")
                                    placeholderTextColor: textTertiary
                                    maximumLength: 6
                                    echoMode: TextInput.Password
                                    inputMethodHints: Qt.ImhDigitsOnly
                                    selectByMouse: true
                                    
                                    background: Rectangle {
                                        color: inputFieldBg
                                        border.color: inputFieldBorderDefault
                                        border.width: 0
                                        Rectangle {
                                            anchors.left: parent.left
                                            anchors.right: parent.right
                                            anchors.bottom: parent.bottom
                                            height: 2
                                            color: {
                                                if (p2pPasswordField.activeFocus) {
                                                    return inputFieldBorderFocus
                                                } else {
                                                    return inputFieldUnderline
                                                }
                                            }
                                        }
                                    }
                                    
                                    onFocusChanged: {
                                        if (focus) {
                                            selectAll()
                                        }
                                    }
                                }
                            }
                        }
                        
                        // P2P连接按钮
                        Rectangle {
                            id: p2pConnectButton
                            Layout.preferredWidth: 80
                            Layout.preferredHeight: 42
                            radius: 6
                            color: p2pConnectMouseArea.containsMouse ? buttonBgHover : buttonBg
                            border.color: buttonBorder
                            border.width: 1.5
                            
                            Text {
                                anchors.centerIn: parent
                                text: qsTr("连接")
                                font.pixelSize: 14
                                font.family: "Microsoft YaHei UI"
                                font.weight: Font.Medium
                                color: buttonText
                            }
                            
                            MouseArea {
                                id: p2pConnectMouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    let code = p2pCodeField.text.trim()
                                    let password = p2pPasswordField.text.trim()
                                    
                                    // 验证输入
                                    if (code.length === 0) {
                                        if (generalDialogLoader.status === Loader.Ready) {
                                            generalDialogLoader.item.iconType = generalDialogLoader.item.error
                                            generalDialogLoader.item.text = qsTr("请输入对方设备代码")
                                            generalDialogLoader.item.buttons = generalDialogLoader.item.ok
                                            generalDialogLoader.item.show()
                                            generalDialogLoader.item.requestActivate()
                                        }
                                        return
                                    }
                                    
                                    if (password.length === 0) {
                                        if (generalDialogLoader.status === Loader.Ready) {
                                            generalDialogLoader.item.iconType = generalDialogLoader.item.error
                                            generalDialogLoader.item.text = qsTr("请输入动态密码")
                                            generalDialogLoader.item.buttons = generalDialogLoader.item.ok
                                            generalDialogLoader.item.show()
                                            generalDialogLoader.item.requestActivate()
                                        }
                                        return
                                    }
                                    
                                    deviceModel.connectViaP2P(code, password)
                                    load_dialog.show(qsTr("正在建立P2P连接..."), qsTr("取消"))
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
                radius: cardRadius
                color: cardBg
                border.color: cardBorder
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
                        text: qsTr("可用设备")
                        font.pixelSize: 13
                        font.family: "Microsoft YaHei UI"
                        font.weight: Font.Medium
                        color: listHeaderText
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    
                    Item { 
                        width: parent.width - availableDevicesText.width - titleText.width
                        height: 1
                    }
                    
                    Text {
                        id: availableDevicesText
                        text: deviceModel ? qsTr("%1 个设备").arg(deviceListView.count) : "0 个设备"
                        font.pixelSize: 12
                        font.family: "Microsoft YaHei UI"
                        color: textDisabled
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
                        text: qsTr("未发现设备")
                        color: textDisabled
                        font.pixelSize: 14
                        visible: deviceListView.count === 0 && (!deviceModel || !deviceModel.scanning)
                    }
                    

                    delegate: Rectangle {
                        id: deviceItem
                        width: deviceListView.width - 2
                        height: 72
                        color: index % 2 === 0 ? deviceItemBg1 : deviceItemBg2
                        radius: 8
                        // anchors.horizontalCenter: parent.horizontalCenter

                        // 定义属性并添加默认值
                        property string deviceName: model.deviceName || qsTr("未知设备")
                        property string deviceIp: model.deviceIP || qsTr("IP未知")
                        property string deviceType: model.deviceType || qsTr("未知")

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
                                color: deviceIconBg
                                Layout.preferredWidth: 48
                                Layout.preferredHeight: 48

                                // 图标内部
                                Rectangle {
                                    width: 40
                                    height: 40
                                    radius: 20
                                    anchors.centerIn: parent
                                    color: deviceItem.containsMouse ? deviceIconInnerHover : deviceIconInner
                                    
                                    Behavior on color {
                                        ColorAnimation { duration: 150 }
                                    }
                                    
                                    Text {
                                        anchors.centerIn: parent
                                        text: deviceItem.deviceName ? deviceItem.deviceName.charAt(0) : "?"
                                        color: textOnPrimary
                                        font.bold: true
                                        font.pixelSize: 16
                                    }
                                    
                                    // 在线状态指示器
                                    Rectangle {
                                        width: 12
                                        height: 12
                                        radius: 6
                                        color: deviceStatusOnline
                                        border.width: 2
                                        border.color: deviceStatusBorder
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
                                    color: textPrimary
                                    elide: Text.ElideRight
                                }

                                Text {
                                    text: deviceItem.deviceIp  // 使用属性而不是直接model访问
                                    font.pixelSize: 13
                                    color: textSecondary
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
                                color: deviceItem.containsMouse ? tagBgHover : tagBg
                                border.color: deviceItem.containsMouse ? tagBorderHover : tagBorder
                                border.width: 1

                                Text {
                                    id: typeTagText
                                    text: deviceItem.deviceType
                                    font.pixelSize: 12
                                    font.family: "Microsoft YaHei UI"
                                    anchors.centerIn: parent
                                    color: deviceItem.containsMouse ? tagTextHover : tagText
                                }
                            }
                        }

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
                                    load_dialog.show(qsTr("等待对方响应"), qsTr("取消"))
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
                        color: progressBarBg
                        
                        // 进度填充
                        Rectangle {
                            id: progressFill
                            width: parent.width * (scanProgress.currentProgress / 100)
                            height: parent.height
                            radius: 3
                            gradient: Gradient {
                                GradientStop { position: 0.0; color: progressBarGradientStart }
                                GradientStop { position: 1.0; color: progressBarGradientEnd }
                            }
                            
                            Behavior on width {
                                NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
                            }
                        }
                    }
                }
                
                // 进度百分比
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
                    color: textAccent
                }
                
                // 状态文本
                Text {
                    id: statusText
                    anchors.left: progressText.visible ? progressText.right : parent.left
                    anchors.leftMargin: 12
                    anchors.right: statusIndicator.left
                    anchors.rightMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    text: {
                        if (!deviceModel) return "模型未加载"
                        if (deviceModel.scanning) return qsTr("正在扫描...")
                        if (deviceListView.count === 0) return qsTr("未发现设备")
                        return qsTr("发现 %1 个设备").arg(deviceListView.count)
                    }
                    font.pixelSize: 13
                    font.family: "Microsoft YaHei UI"
                    color: textSecondary
                    elide: Text.ElideRight
                }
                
                // 状态指示器容器
                Item {
                    id: statusIndicator
                    width: 24
                    height: 24
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    
                    // 加载指示器
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
                                    ctx.strokeStyle = spinnerColor
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
                    
                    // 完成状态
                    Rectangle {
                        id: completeStatus
                        anchors.fill: parent
                        radius: 12
                        color: completeStatusBg
                        visible: deviceModel && !deviceModel.scanning
                        
                        Text {
                            anchors.centerIn: parent
                            text: "✓"
                            font.pixelSize: 14
                            color: completeStatusText
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