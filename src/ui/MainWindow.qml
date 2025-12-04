import QtQuick
import QtQuick.Window
import Qt.labs.platform
import QtQuick.Controls

ApplicationWindow  {
    id: root
    width: Screen.width * 0.5
    height: Math.max(60, Screen.height * 0.08)
    visible: true
    flags: Qt.Window | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint
    color: "transparent"

    x: (Screen.width - width) / 2
    y: -height + 4

    property bool expanded: false
    property int animationDuration: 300
    property bool dragActive: false
    property bool mouseIsInWindow: false
    property int itemWidth: 100
    property int itemHeight: 80
    property int itemsPerRow: Math.max(1, Math.floor((width - 40) / itemWidth))

    // 连接状态属性
    property string current_device: ""
    property bool isConnected: false
    property string connectionStatus: isConnected ? current_device : "未连接"

    property var currentAcceptHandler: null
    property var currentRejectHandler: null

    property color primaryColor: "#6366F1"    // 主色调
    property color secondaryColor: "#8B5CF6"  // 次要色调
    property color accentColor: "#EC4899"     // 强调色
    property color successColor: "#10B981"    // 成功色
    property color warningColor: "#F59E0B"    // 警告色
    property color dangerColor: "#EF4444"     // 危险色
    property color infoColor: "#3B82F6"       // 信息色
    
    property color bgColor: "#FFFFFF"
    property color cardColor: "#F8FAFC"
    property color borderColor: "#E2E8F0"
    property color textPrimary: "#1E293B"
    property color textSecondary: "#64748B"
    property color textLight: "#94A3B8"

    Loader {
        id: deviceWindowLoader
        source: "qrc:/qml/ui/DeviceListWindow.qml"
        
        onLoaded: {
            item.deviceModel = device_list_model
        }
    }
    
    Loader {
        id: connectRequestLoader  
        source: "qrc:/qml/ui/ConnectRequestDialog.qml"
        
        onLoaded: {
            item.connection_model = connection_manager
        }
    }

    Connections {
        target: connection_manager
        enabled: connectRequestLoader.status === Loader.Ready
        
        function onHaveConnectError(message) {
            if (generalDialogLoader.status === Loader.Ready) {
                generalDialogLoader.item.iconType = generalDialogLoader.item.error
                generalDialogLoader.item.text = message
                generalDialogLoader.item.buttons = generalDialogLoader.item.ok
                generalDialogLoader.item.show()
                generalDialogLoader.item.requestActivate()
            }
        }
        
        function onHaveRecvError(message) {
            if (generalDialogLoader.status === Loader.Ready) {
                generalDialogLoader.item.iconType = generalDialogLoader.item.error
                generalDialogLoader.item.text = message
                generalDialogLoader.item.buttons = generalDialogLoader.item.ok
                generalDialogLoader.item.show()
                generalDialogLoader.item.requestActivate()
                resetStatus()
            }
        }
        
        function onPeerClosed() {
            if (generalDialogLoader.status === Loader.Ready && isConnected) {
                generalDialogLoader.item.iconType = generalDialogLoader.item.error
                generalDialogLoader.item.text = "对方断开连接"
                generalDialogLoader.item.buttons = generalDialogLoader.item.ok
                generalDialogLoader.item.show()
                generalDialogLoader.item.requestActivate()
                resetStatus()
            }
            resetStatus()
        }
    }

    Loader {
        id: generalDialogLoader
        source: "qrc:/qml/ui/GeneralDialog.qml"
        onLoaded: {
            item.accepted.connect(function() {
                if (currentAcceptHandler) {
                    currentAcceptHandler()
                }
                currentAcceptHandler = null
                currentRejectHandler = null
            })
            
            item.rejected.connect(function() {
                if (currentRejectHandler) {
                    currentRejectHandler()
                }
                currentAcceptHandler = null
                currentRejectHandler = null
            })
        }
    }

    // 现代化背景
    Rectangle {
        id: mainBackground
        anchors.fill: parent
        radius: 20
        color: dragActive ? "#E0E7FF" : bgColor
        border.color: dragActive ? primaryColor : borderColor
        border.width: 1
        
        // 现代感渐变
        gradient: Gradient {
            GradientStop { position: 0.0; color: dragActive ? "#E0E7FF" : "#F8FAFC" }
            GradientStop { position: 1.0; color: dragActive ? "#C7D2FE" : bgColor }
        }
    }

    // 顶部装饰线
    Rectangle {
        width: 40
        height: 3
        radius: 1.5
        color: primaryColor
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 8
        visible: root.expanded
    }

    // 主窗口的拖拽区域
    DropArea {
        anchors.fill: parent
        onEntered: function(drag) {
            if (drag.hasUrls) {
                drag.accept()
                dragActive = true
                if (!root.expanded) {
                    root.expanded = true
                }
            }
        }
        onExited: {
            dragActive = false
        }
        onDropped: function(drop) {
            dragActive = false
            if (drop.hasUrls && drop.urls) {
                var newFiles = []
                for (var i = 0; i < drop.urls.length; i++) {
                    var fileUrl = drop.urls[i].toString()
                    newFiles.push(fileUrl)
                }
                file_list_model.addFiles(newFiles, false);
                drop.accept()
                
                extendCollapseTime()
            } else {
                console.log("没有检测到文件URL")
            }
        }
    }

    // 鼠标区域
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true

        onEntered: {
            collapseTimer.stop()
            if (!root.expanded) {
                root.expanded = true
            }
        }

        onExited: {            
            if (!root.mouseIsInWindow) {
                collapseTimer.start()
            }
        }
    }

    // 延迟收缩
    Timer {
        id: collapseTimer
        interval: 500
        onTriggered: {
            if (root.expanded) {
                root.expanded = false
            }
            resetCollapseTime()
        }
    }

    function extendCollapseTime() {
        collapseTimer.stop()
        collapseTimer.interval = 3000
        collapseTimer.start()
    }

    function resetCollapseTime() {
        collapseTimer.interval = 500
    }

    // 顶部触发窗口
    Window {
        id: triggerWindow
        width: root.width
        height: 6
        x: root.x
        y: 0
        visible: root.visible
        flags: Qt.FramelessWindowHint | Qt.Tool | Qt.WindowStaysOnTopHint
        color: "transparent"

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: {
                if (!root.expanded) {
                    root.expanded = true
                }
            }
            onExited: {
                if (!root.mouseInWindow) {
                    collapseTimer.start()
                }
            }
        }

        DropArea {
            anchors.fill: parent
            onEntered: function(drag){
                if (!root.expanded) {
                    root.expanded = true
                }
                if (drag.hasUrls) {
                    drag.accept()
                    collapseTimer.stop()
                }
            }
            onDropped: function(drop) {
                if (drop.hasUrls && drop.urls) {
                    var newFiles = []
                    for (var i = 0; i < drop.urls.length; i++) {
                        var fileUrl = drop.urls[i].toString()
                        newFiles.push(fileUrl)
                    }
                    file_list_model.addFiles(newFiles,false);
                    drop.accept()
                    extendCollapseTime()
                } else {
                    console.log("触发窗口: 没有检测到文件URL")
                }
            }
        }
    }

    // 动画
    Behavior on y {
        NumberAnimation {
            duration: root.animationDuration
            easing.type: Easing.OutCubic
        }
    }

    onExpandedChanged: {
        if (expanded) {
            root.y = 0
            updateWindowHeight()
            scrollToBottom()
        } else {
            root.y = -root.height + 4
        }
    }

    function updateWindowHeight() {
        if (root.expanded) {
            if (file_list_model.getFileCount() > 0) {
                var rowsNeeded = Math.ceil(file_list_model.getFileCount() / itemsPerRow)
                var visibleRows = Math.min(rowsNeeded, 1.5)
                root.height = 40 + (visibleRows * itemHeight) + 20
            } else {
                root.height = Math.max(60, Screen.height * 0.08)
            }
        }
    }

    function scrollToBottom() {
        if (fileGridView.count > 0) {
            fileGridView.positionViewAtEnd()
            scrollTimer.restart()
        }
    }

    Timer {
        id: scrollTimer
        interval: 50
        onTriggered: {
            fileGridView.positionViewAtEnd()
        }
    }

    onWidthChanged: {
        itemsPerRow = Math.max(1, Math.floor((width - 40) / itemWidth))
        updateWindowHeight()
    }

    // 文件网格视图
    GridView {
        id: fileGridView
        anchors {
            top: parent.top
            topMargin: 40
            left: parent.left
            leftMargin: 20
            right: parent.right
            rightMargin: 20
            bottom: parent.bottom
            bottomMargin: 10
        }
        clip: true
        model: file_list_model
        cellWidth: itemWidth
        cellHeight: itemHeight
        
        onCountChanged: {
            updateWindowHeight()
            scrollToBottom()
        }

        delegate: Item {
            width: itemWidth - 5
            height: itemHeight - 5
            
            Rectangle {
                id: fileCard
                anchors.fill: parent
                radius: 12
                color: index % 2 === 0 ? Qt.lighter(primaryColor, 3.5) : cardColor
                border.color: index % 2 === 0 ? Qt.darker(primaryColor, 1.2) : borderColor
                border.width: 1
                
                Rectangle {
                    anchors.fill: parent
                    anchors.margins: -1
                    radius: 13
                    color: "transparent"
                    border.color: "#10000000"
                    border.width: 1
                    z: -1
                }
            }

            // 右键菜单
            Menu {
                id: contextMenu
                MenuItem {
                    text: "打开文件"
                    enabled: model.fileStatus === file_list_model.StatusCompleted | !model.isRemote
                    onTriggered: {
                        if (model.fileUrl) {
                            Qt.openUrlExternally(model.fileUrl)
                        } else if (model.filePath) {
                            var fileUrl = model.filePath.startsWith("file://") ? model.filePath : "file:///" + model.filePath
                            Qt.openUrlExternally(fileUrl)
                        }
                    }
                }
                
                MenuSeparator {}
                
                MenuItem {
                    text: "复制文件名"
                    onTriggered: {
                        file_list_model.copyText(model.fileName)
                    }
                }
                
                MenuItem {
                    text: "复制文件路径"
                    enabled: model.filePath
                    onTriggered: {
                        if (model.filePath) {
                            file_list_model.copyText(model.filePath)
                        }
                    }
                }
                
                MenuSeparator {}
                
                MenuItem {
                    text: "下载文件"
                    enabled: model.isRemote && model.fileStatus !== file_list_model.StatusDownloading && model.fileStatus !==file_list_model.StatusCompleted
                    onTriggered: {
                        file_list_model.downloadFile(index)
                    }
                }
                
                MenuItem {
                    text: "刷新"
                    onTriggered: {
                        // 重新上传文件
                    }
                }
                
                MenuSeparator {}
                
                MenuItem {
                    text: "删除"
                    onTriggered: {
                        file_list_model.removeFile(index)
                    }
                }
                
                MenuItem {
                    text: "属性"
                    onTriggered: {
                        // showFileProperties(index)
                    }
                }
            }
            
            // 文件项的拖拽源
            Drag.active: fileDragArea.drag.active && (model.fileStatus === 1 || model.fileStatus === 5 || model.fileStatus === 6)
            Drag.dragType: Drag.Automatic
            Drag.supportedActions: Qt.CopyAction
            Drag.mimeData: {
                "text/uri-list": [model.fileUrl],
                "text/plain": model.filePath
            }

            ToolTip {
                id: fileToolTip
                visible: fileDragArea.containsMouse
                text: model.toolTip
                delay: 1500
                timeout: -1
                
                background: Rectangle {
                    radius: 8
                    color: "transparent"
                    
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#1E293B" }
                        GradientStop { position: 1.0; color: "#0F172A" }
                    }
                    
                    border.width: 1
                    border.color: primaryColor
                }
                
                // 文字样式
                contentItem: Text {
                    text: fileToolTip.text
                    font.pixelSize: 11
                    font.family: "Microsoft YaHei UI"
                    color: "#E2E8F0"
                    wrapMode: Text.WordWrap
                    maximumLineCount: 3
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    lineHeight: 1.3
                }
                
                // 位置调整
                y: -height - 8
                x: (parent.width - width) / 2
            }

            Column {
                anchors.centerIn: parent
                width: parent.width - 20
                spacing: 1
                
                // 图标背景
                Rectangle {
                    id: iconBg
                    width: 42
                    height: 42
                    radius: 8
                    color: index % 2 === 0 ? Qt.rgba(255, 255, 255, 0.9) : Qt.rgba(99, 102, 241, 0.1)
                    border.color: index % 2 === 0 ? Qt.rgba(99, 102, 241, 0.3) : Qt.rgba(99, 102, 241, 0.2)
                    border.width: 1
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.topMargin: 4

                    Image {
                        width: 28
                        height: 28
                        source: model.fileIcon
                        fillMode: Image.PreserveAspectFit
                        anchors.centerIn: parent
                    }
                }
                
                // 文件名
                Text {
                    text: model.fileName
                    font.pixelSize: 11
                    font.bold: true
                    color: textPrimary
                    width: parent.width - 4
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.NoWrap
                    maximumLineCount: 1
                    elide: Text.ElideMiddle
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                
                // 状态行
                Row {
                    width: parent.width - 10
                    spacing: 4
                    anchors.horizontalCenter: parent.horizontalCenter
                    height: 14
                    
                    // 状态指示器
                    Rectangle {
                        id: statusIndicator
                        width: 8
                        height: 8
                        radius: 4
                        visible: model.isRemote || (model.fileStatus === 3 && model.fileProgress != 100) || model.fileStatus === 5
                        anchors.verticalCenter: parent.verticalCenter
                        
                        color: {
                            switch(model.fileStatus) {
                                case 0: return warningColor
                                case 1: return textLight
                                case 2: return textLight
                                case 3: return infoColor
                                case 4: return accentColor
                                case 5: return secondaryColor
                                case 6: return successColor
                                case 7: return dangerColor
                                default: return textLight
                            }
                        }
                    }
                    
                    // 进度条背景
                    Rectangle {
                        id: progressBarBg
                        width: parent.width - statusIndicator.width - parent.spacing - 18
                        height: 4
                        radius: 2
                        color: "#E2E8F0"
                        anchors.verticalCenter: parent.verticalCenter
                        visible: (model.fileStatus === 3 || model.fileStatus === 4) && model.fileProgress != 100
                        
                        // 进度条
                        Rectangle {
                            width: Math.max(0, parent.width * (model.fileProgress / 100.0))
                            height: parent.height
                            radius: 2
                            color: statusIndicator.color
                        }
                    }
                    
                    // 进度百分比
                    Text {
                        text: qsTr("%1%").arg(model.fileProgress)
                        font.pixelSize: 9
                        font.bold: true
                        color: textSecondary
                        anchors.verticalCenter: parent.verticalCenter
                        visible: (model.fileStatus === 3 || model.fileStatus === 4) && model.fileProgress != 100
                    }
                }
            }
            
            Item {
                id: dragProxy
                width: 1; height: 1 
                visible: false

                Drag.active: false
                Drag.dragType: Drag.Automatic
                Drag.supportedActions: Qt.CopyAction
                Drag.mimeData: {
                    "text/uri-list": model.fileUrl ? [model.fileUrl.toString()] : [],
                    "text/plain": model.filePath || ""
                }
                Drag.imageSource: model.fileIcon
            }
            
            // 文件拖拽区域
            MouseArea {
                id: fileDragArea
                anchors.fill: parent
                enabled: true
                hoverEnabled: true
                acceptedButtons: Qt.LeftButton | Qt.RightButton

                drag.target: dragProxy
                drag.axis: Drag.XandYAxis

                onPressed: function(mouse) {
                    if (mouse.button === Qt.RightButton) {
                        contextMenu.open()
                    } else if (mouse.button === Qt.LeftButton) {
                        if (model.fileStatus === file_list_model.StatusCompleted || !model.isRemote) {
                            dragProxy.Drag.active = true
                        }
                    }
                }

                onPositionChanged: function(mouse) {
                    if (dragProxy.visible && dragProxy.Drag.active) {
                        var globalPos = mapToGlobal(mouse.x, mouse.y)
                        var itemPos = root.contentItem.mapFromGlobal(globalPos.x, globalPos.y)
                        dragProxy.x = itemPos.x - dragProxy.width / 2
                        dragProxy.y = itemPos.y - dragProxy.height / 2
                    }
                }
                
                onReleased: {
                    dragProxy.visible = false
                    dragProxy.Drag.active = false
                }
                
                onDoubleClicked: {
                    if (model.fileUrl) {
                        Qt.openUrlExternally(model.fileUrl)
                    } else if (model.filePath) {
                        var fileUrl = model.filePath.startsWith("file://") ? model.filePath : "file:///" + model.filePath
                        Qt.openUrlExternally(fileUrl)
                    }
                }
                
                onEntered: {
                    mouseIsInWindow = true
                    fileCard.border.width = 2
                    fileCard.border.color = primaryColor
                }
                onExited: {
                    mouseIsInWindow = false
                    fileCard.border.width = 1
                    fileCard.border.color = index % 2 === 0 ? Qt.darker(primaryColor, 1.2) : borderColor
                }
            }
            
            // 删除按钮
            Rectangle {
                id: deleteButton
                width: 16
                height: 16
                radius: 8
                color: deleteMouseArea.containsMouse ? dangerColor : "transparent"
                border.color: deleteMouseArea.containsMouse ? dangerColor : "#CBD5E1"
                border.width: 1
                anchors {
                    top: parent.top
                    topMargin: 3
                    right: parent.right
                    rightMargin: 3
                }
                
                Canvas {
                    anchors.fill: parent
                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.reset()
                        ctx.strokeStyle = deleteMouseArea.containsMouse ? "white" : "#64748B"
                        ctx.lineWidth = 1.5
                        ctx.lineCap = "round"
                        
                        var centerX = width / 2
                        var centerY = height / 2
                        var halfSize = 3
                        
                        // 绘制第一条斜线（从左上到右下）
                        ctx.beginPath()
                        ctx.moveTo(centerX - halfSize, centerY - halfSize)
                        ctx.lineTo(centerX + halfSize, centerY + halfSize)
                        ctx.stroke()
                        
                        // 绘制第二条斜线（从右上到左下）
                        ctx.beginPath()
                        ctx.moveTo(centerX + halfSize, centerY - halfSize)
                        ctx.lineTo(centerX - halfSize, centerY + halfSize)
                        ctx.stroke()
                    }
                    
                }
                
                MouseArea {
                    id: deleteMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    
                    onClicked: {
                        file_list_model.removeFile(index)
                    }
                    
                    onEntered: {
                        deleteButton.children[0].requestPaint()
                    }
                    
                    onExited: {
                        deleteButton.children[0].requestPaint()
                    }
                }
            }
        }
        
        // 空列表提示
        Text {
            anchors.centerIn: parent
            text: "📁 暂无文件，拖放文件到此处"
            font.pixelSize: 14
            color: "#7f8c8d"
            visible: fileGridView.count === 0
        }
    }

    Rectangle {
        id: titleBar
        width: parent.width
        height: 40
        color: "transparent"
        
        // 装饰线
        Rectangle {
            width: 4
            height: 18
            radius: 2
            color: primaryColor
            anchors {
                left: parent.left
                leftMargin: 18
                verticalCenter: parent.verticalCenter
            }
        }
        
        // 标题
        Row {
            id: titleRow
            spacing: 8
            anchors {
                left: parent.left
                leftMargin: 30
                verticalCenter: parent.verticalCenter
            }
            
            Image {
                source: "qrc:/logo/logo.png"
                width: 16
                height: 16
                anchors.verticalCenter: parent.verticalCenter
                fillMode: Image.PreserveAspectFit
            }
            
            Text {
                id: titleText
                text: dragActive ? "释放以添加文件" : "XFileTransit"
                font.pixelSize: 14
                font.bold: true
                color: textPrimary
                anchors.verticalCenter: parent.verticalCenter
            }
        }
        
        // 连接状态容器 - 在标题的右侧
        Row {
            id: connectionContainer
            spacing: 8
            anchors {
                left: titleRow.right
                leftMargin: 20
                verticalCenter: parent.verticalCenter
            }
            
            // 状态点
            Rectangle {
                width: 8
                height: 8
                radius: 4
                color: root.isConnected ? successColor : dangerColor
                anchors.verticalCenter: parent.verticalCenter
            }
            
            // 状态文本
            Text {
                id: connectionStatusText
                text: root.connectionStatus
                font.pixelSize: 12
                color: root.isConnected ? successColor : dangerColor
                anchors.verticalCenter: parent.verticalCenter
            }
            
            // 现代化连接按钮
            Rectangle {
                id: switchButton
                width: 55
                height: 24
                radius: 12
                color: switchMouseArea.containsMouse ? 
                       (root.isConnected ? dangerColor : successColor) : 
                       "#F1F5F9"
                border.color: switchMouseArea.containsMouse ? 
                             Qt.darker(root.isConnected ? dangerColor : successColor, 1.2) : 
                             borderColor
                border.width: 1
                anchors.verticalCenter: parent.verticalCenter
                
                Text {
                    text: isConnected ? "断开" : "连接"
                    font.pixelSize: 11
                    font.bold: true
                    color: switchMouseArea.containsMouse ? "white" : textSecondary
                    anchors.centerIn: parent
                }
                
                MouseArea {
                    id: switchMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        if(!isConnected){
                            if (deviceWindowLoader.status === Loader.Ready) {
                                deviceWindowLoader.item.show()
                                deviceWindowLoader.item.requestActivate()
                            } else {
                                console.error("设备窗口未正确加载:", deviceWindowLoader.status)
                            }
                        } else {
                            if (generalDialogLoader.status === Loader.Ready) {
                                generalDialogLoader.item.iconType = generalDialogLoader.item.info
                                generalDialogLoader.item.text = "确定断开连接？"
                                generalDialogLoader.item.buttons = generalDialogLoader.item.yes | generalDialogLoader.item.no
                                
                                root.currentAcceptHandler = function() {
                                    resetStatus()
                                    connection_manager.disconnect()
                                }
                                generalDialogLoader.item.show()
                                generalDialogLoader.item.requestActivate()
                            }
                        }
                    }
                    onEntered: {
                        mouseIsInWindow = true
                    }
                    onExited: {
                        mouseIsInWindow = false
                    }
                }
            }
            
            Connections {
                target: connection_manager
                function onHaveConRequest(device_ip, device_name) {
                    if (connectRequestLoader.status === Loader.Ready) {
                        connectRequestLoader.item.device_ip = device_ip
                        connectRequestLoader.item.device_name = device_name
                        connectRequestLoader.item.show()
                        connectRequestLoader.item.requestActivate()
                    } else {
                        console.error("连接请求对话框未正确加载:", connectRequestLoader.status)
                    }
                }
            }
            
            Connections {
                target: connectRequestLoader.item
                enabled: connectRequestLoader.status === Loader.Ready
                
                function onAccepted(ip, name) {
                    current_device = (name == "UnKnown" ? ip : name);
                    isConnected = true;
                    if(fileGridView.count){
                        if (generalDialogLoader.status === Loader.Ready) {
                            generalDialogLoader.item.iconType = generalDialogLoader.item.info
                            generalDialogLoader.item.text = "是否同步当前文件"
                            generalDialogLoader.item.buttons = generalDialogLoader.item.yes | generalDialogLoader.item.no
                                
                            root.currentAcceptHandler = function() {
                                file_list_model.syncCurrentFiles()
                            }
                            generalDialogLoader.item.show()
                            generalDialogLoader.item.requestActivate()
                        }
                    }
                }
                
                function onRejected(ip, name) {
                }
            }
            
            Connections {
                target: device_list_model
                enabled: deviceWindowLoader.status === Loader.Ready
                
                function onConnectResult(ret, ip) {
                    if(ret){
                        current_device = ip;
                        isConnected = true;
                        if(fileGridView.count > 0){
                            // 有文件时询问是否同步
                            if (generalDialogLoader.status === Loader.Ready) {
                                generalDialogLoader.item.iconType = generalDialogLoader.item.info
                                generalDialogLoader.item.text = "是否同步当前文件"
                                generalDialogLoader.item.buttons = generalDialogLoader.item.yes | generalDialogLoader.item.no
                                
                                root.currentAcceptHandler = function() {
                                    file_list_model.syncCurrentFiles()
                                }
                                generalDialogLoader.item.show()
                                generalDialogLoader.item.requestActivate()
                            }
                        } else {
                            // 没有文件时显示连接成功提示
                            if (generalDialogLoader.status === Loader.Ready) {
                                generalDialogLoader.item.iconType = generalDialogLoader.item.success
                                generalDialogLoader.item.text = "连接成功"
                                generalDialogLoader.item.buttons = generalDialogLoader.item.ok
                                
                                root.currentAcceptHandler = null
                                root.currentRejectHandler = null
                                
                                generalDialogLoader.item.show()
                                generalDialogLoader.item.requestActivate()
                            }
                        }
                    }else{
                        if (generalDialogLoader.status === Loader.Ready) {
                            generalDialogLoader.item.iconType = generalDialogLoader.item.error
                            generalDialogLoader.item.text = "连接被拒绝"
                            generalDialogLoader.item.buttons = generalDialogLoader.item.ok
                                
                            root.currentAcceptHandler = null
                            root.currentRejectHandler = null
                                
                            generalDialogLoader.item.show()
                            generalDialogLoader.item.requestActivate()
                        }
                    }
                }
            }
        }        
        
        // 清空按钮
        Rectangle {
            id: clearButton
            width: 55
            height: 24
            radius: 12
            color: clearMouseArea.containsMouse ? dangerColor : "#F1F5F9"
            border.color: clearMouseArea.containsMouse ? Qt.darker(dangerColor, 1.2) : borderColor
            border.width: 1
            visible: root.expanded && fileGridView.count > 0
            anchors {
                right: closeButton.left
                rightMargin: 30
                verticalCenter: parent.verticalCenter
            }
            
            Text {
                text: "清空"
                font.pixelSize: 11
                font.bold: true
                color: clearMouseArea.containsMouse ? "white" : textSecondary
                anchors.centerIn: parent
            }
            
            MouseArea {
                id: clearMouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    file_list_model.clearAll()
                }
                onEntered: {
                    mouseIsInWindow = true
                }
                onExited: {
                    mouseIsInWindow = false
                }
            }
        }
        
        // 关闭按钮
        Rectangle {
            id: closeButton
            width: 24
            height: 24
            radius: 12
            color: closeMouseArea.containsMouse ? dangerColor : "#F1F5F9"
            border.color: closeMouseArea.containsMouse ? Qt.darker(dangerColor, 1.2) : borderColor
            border.width: 1
            visible: root.expanded
            anchors {
                right: parent.right
                rightMargin: 12
                verticalCenter: parent.verticalCenter
            }

            Text {
                text: "×"
                font.pixelSize: 14
                font.bold: true
                color: closeMouseArea.containsMouse ? "white" : textSecondary
                anchors.centerIn: parent
            }

            MouseArea {
                id: closeMouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: Qt.quit()
                onEntered: {
                    mouseIsInWindow = true
                }
                onExited: {
                    mouseIsInWindow = false
                }
            }
        }
    }

    // 收缩时显示的小条
    Rectangle {
        width: parent.width
        height: 4
        color: primaryColor
        border.color: Qt.darker(primaryColor, 1.2)
        border.width: 1
        y: parent.height - 4
        radius: 2
        visible: !root.expanded
    }
    
    Component.onDestruction: {
    }

    function resetStatus() {
        isConnected = false
        current_device = ""
    }
}