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
    property int itemWidth: 120  // 每个文件项的宽度
    property int itemHeight: 80  // 每个文件项的高度
    property int itemsPerRow: Math.max(1, Math.floor((width - 40) / itemWidth)) // 每行显示的文件数量

    // 连接状态属性
    property bool isConnected: false
    property string connectionStatus: isConnected ? "已连接" : "未连接"
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
                
                // 添加文件后延长收缩时间
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

    // 添加文件后的延长收缩时间
    function extendCollapseTime() {
        collapseTimer.stop()
        collapseTimer.interval = 3000  // 延长到3秒
        collapseTimer.start()
    }

    // 重置收缩时间到默认值
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

        // 拖拽进入时展开
        DropArea {
            anchors.fill: parent
            onEntered: function(drag){
                console.log("触发窗口: 拖拽进入, hasUrls:", drag.hasUrls)
                if (!root.expanded) {
                    root.expanded = true
                }
                if (drag.hasUrls) {
                    drag.accept()
                    collapseTimer.stop()
                }
            }
            onDropped: function(drop) {
                console.log("触发窗口: 文件拖放, hasUrls:", drop.hasUrls, "urls count:", drop.urls ? drop.urls.length : 0)
                if (drop.hasUrls && drop.urls) {
                    var newFiles = []
                    for (var i = 0; i < drop.urls.length; i++) {
                        var fileUrl = drop.urls[i].toString()
                        newFiles.push(fileUrl)
                    }
                    file_list_model.addFiles(newFiles,false);

                    // 使用主窗口的添加函数处理重复文件
                    // addFilesToList(newFiles)
                    drop.accept()
                    
                    // 添加文件后延长收缩时间
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
            // 展开时滚动到底部
            if (fileGridView.count > 0) {
                Qt.callLater(scrollToBottom)
            }
        } else {
            root.y = -root.height + 4
        }
    }

    // 更新窗口高度
    function updateWindowHeight() {
        if (root.expanded) {
            if (file_list_model.getFileCount() > 0) {
                var rowsNeeded = Math.ceil(file_list_model.getFileCount() / itemsPerRow)
                var visibleRows = Math.min(rowsNeeded, 1.5) // 显示一行半
                root.height = 40 + (visibleRows * itemHeight) + 20
            } else {
                root.height = Math.max(60, Screen.height * 0.08)
            }
        }
    }

    // 滚动到底部
    function scrollToBottom() {
        if (fileGridView.count > 0) {
            var lastItemIndex = fileGridView.count - 1
            fileGridView.positionViewAtEnd()
            // 使用Timer确保在布局完成后滚动
            scrollTimer.restart()
        }
    }

    Timer {
        id: scrollTimer
        interval: 10
        onTriggered: {
            fileGridView.positionViewAtEnd()
        }
    }

    onWidthChanged: {
        itemsPerRow = Math.max(1, Math.floor((width - 40) / itemWidth))
        updateWindowHeight()
    }

    // 背景
    Rectangle {
        anchors.fill: parent
        radius: 20
        color: dragActive ? "#88A8DFF7" : "#CCF0F0F0"
        border.color: "#40000000"
        border.width: 1
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
            console.log("文件数量变化，当前数量:", count)
            updateWindowHeight()
        }

        delegate: Rectangle {
            width: itemWidth - 5
            height: itemHeight - 5
            color: index % 2 === 0 ? "#E8F4FD" : "#FFFFFF"
            radius: 8
            border.color: "#40000000"
            border.width: 1
            
            // 文件项的拖拽源
            Drag.active: fileDragArea.drag.active
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
                delay: 500
                timeout: 5000
            }

            Column {
                anchors.centerIn: parent
                width: parent.width - 20
                spacing: 5
                Image {
                    width: 50
                    height: 50
                    source: model.fileIcon
                    fillMode: Image.PreserveAspectFit
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                
                Text {
                    text: model.fileName
                    font.pixelSize: 11
                    color: "#2c3e50"
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.NoWrap
                    maximumLineCount: 1
                    elide: Text.ElideMiddle
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
            
            // 文件拖拽区域
            MouseArea {
                id: fileDragArea
                anchors.fill: parent
                drag.target: null
                enabled: true
                hoverEnabled: true
                onPressed: {
                    // 创建拖拽可视化项
                    fileDragItem.parent = root.contentItem
                    var globalPos = mapToGlobal(mouseX, mouseY)
                    var itemPos = root.contentItem.mapFromGlobal(globalPos.x, globalPos.y)
                    fileDragItem.x = itemPos.x - fileDragItem.width / 2
                    fileDragItem.y = itemPos.y - fileDragItem.height / 2
                    fileDragItem.visible = true
                    
                    fileDragItem.Drag.active = true
                }
                
                onPositionChanged: {
                    if (drag.active) {
                        fileDragItem.x = mapToItem(root, mouseX, mouseY).x - fileDragItem.width / 2
                        fileDragItem.y = mapToItem(root, mouseX, mouseY).y - fileDragItem.height / 2
                    }
                }
                
                onReleased: {
                    fileDragItem.visible = false
                    fileDragItem.Drag.active = false
                }
                
                // 双击打开文件
                onDoubleClicked: {
                    if (model.fileUrl) {
                        Qt.openUrlExternally(model.fileUrl)
                    } else if (model.filePath) {
                        var fileUrl = model.filePath.startsWith("file://") ? model.filePath : "file:///" + model.filePath
                        Qt.openUrlExternally(fileUrl)
                    }
                }
                onEntered:{
                    mouseIsInWindow = true
                }
                onExited:{
                    mouseIsInWindow = false
                }
            }
            
            // 拖拽可视化项
            Rectangle {
                id: fileDragItem
                visible: false

                // 拖拽支持
                Drag.active: false
                Drag.dragType: Drag.Automatic
                Drag.supportedActions: Qt.CopyAction
                Drag.mimeData: {
                    "text/uri-list": model.fileUrl ? [model.fileUrl.toString()] : [],
                    "text/plain": model.filePath || model.fileUrl || ""
                }
                Drag.hotSpot.x: width / 2
                Drag.hotSpot.y: height / 2
                
                // 拖拽提示
                Text {
                    anchors {
                        top: parent.top
                        right: parent.right
                        margins: 5
                    }
                    text: "📄"
                }
            }
            
            // 删除按钮
            Rectangle {
                id: deleteButton
                width: 20
                height: 20
                radius: 10
                color: deleteMouseArea.containsMouse ? "#ff6b6b" : "transparent"
                border.color: "#40000000"
                border.width: 1
                anchors {
                    top: parent.top
                    topMargin: 5
                    right: parent.right
                    rightMargin: 5
                }
                
                Text {
                    text: "×"
                    font.pixelSize: 12
                    font.bold: true
                    color: deleteMouseArea.containsMouse ? "white" : "#666666"
                    anchors.centerIn: parent
                }
                
                MouseArea {
                    id: deleteMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    
                    onClicked: {
                        // 从列表中移除文件
                        file_list_model.removeFile(index)
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

    // 标题栏
    Rectangle {
        id: titleBar
        width: parent.width
        height: 40
        color: "transparent"
        
        Text {
            id: titleText
            text: dragActive ? "🔄 释放文件以处理" : "🔄 文件中转站"
            font.pixelSize: 14
            font.bold: true
            color: "#2c3e50"
            anchors {
                left: parent.left
                leftMargin: 20
                verticalCenter: parent.verticalCenter
            }
        }
       // 连接状态和切换按钮容器
        Row {
            id: connectionContainer
            spacing: 8
            anchors {
                left: titleText.right
                leftMargin: 15
                verticalCenter: parent.verticalCenter
            }
            
            // 连接状态文本
            Text {
                id: connectionStatusText
                text: root.connectionStatus
                font.pixelSize: 12
                color: root.isConnected ? "#27ae60" : "#e74c3c"
                anchors.verticalCenter: parent.verticalCenter
            }
            
            // 切换按钮
            Rectangle {
                id: switchButton
                width: 50
                height: 24
                radius: 12
                color: switchMouseArea.containsMouse ? (root.isConnected ? "#e74c3c" : "#27ae60") : "#CCCCCC"
                border.color: "#40000000"
                border.width: 1
                anchors.verticalCenter: parent.verticalCenter
                
                Text {
                    text: "切换" 
                    font.pixelSize: 10
                    color: "white"
                    font.bold: true
                    anchors.centerIn: parent
                }
                
                MouseArea {
                    id: switchMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        var deviceWindow = Qt.createComponent("qrc:/qml/ui/DeviceListWindow.qml")
                        if (deviceWindow.status === Component.Ready) {
                            var window = deviceWindow.createObject(root, {
                                "deviceModel": device_list_model
                            })
                            window.show()
                            window.requestActivate()
                        } else {
                            console.error("无法创建设备列表窗口:", deviceWindow.errorString())
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
                    var con_request_dialog = Qt.createComponent("qrc:/qml/ui/ConnectRequestDialog.qml")
                    if (con_request_dialog.status === Component.Ready) {
                        var window = con_request_dialog.createObject(root, {
                            "device_ip": device_ip,"device_name":device_name
                        })

                        window.show()
                        window.requestActivate()
                    } else {
                        console.error("无法创建连接请求弹窗:", con_request_dialog.errorString())
                    }
                }
            }
        }        
        // 清空按钮
        Rectangle {
            id: clearButton
            width: 60
            height: 24
            radius: 12
            color: clearMouseArea.containsMouse ? "#e74c3c" : "transparent"
            border.color: "#40000000"
            border.width: 1
            visible: root.expanded && fileGridView.count > 0
            anchors {
                right: closeButton.left
                rightMargin: 10
                verticalCenter: parent.verticalCenter
            }
            
            Text {
                text: "清空"
                font.pixelSize: 12
                color: clearMouseArea.containsMouse ? "white" : "#666666"
                anchors.centerIn: parent
            }
            
            MouseArea {
                id: clearMouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    file_list_model.clearAll()
                }
                onEntered:{
                    mouseIsInWindow = true
                }
                onExited:{
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
            color: closeMouseArea.containsMouse ? "#E81123" : "transparent"
            border.color: "#40000000"
            border.width: 1
            visible: root.expanded
            anchors {
                top: parent.top
                topMargin: 8
                right: parent.right
                rightMargin: 8
                verticalCenter: parent.verticalCenter
            }

            Text {
                text: "×"
                font.pixelSize: 16
                font.bold: true
                color: closeMouseArea.containsMouse ? "white" : "#666666"
                anchors.centerIn: parent
            }

            MouseArea {
                id: closeMouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: Qt.quit()
                onEntered:{
                    mouseIsInWindow = true
                }
                onExited:{
                    mouseIsInWindow = false
                }
            }
        }
    }

    // 收缩时顶部条
    Rectangle {
        width: parent.width
        height: 4
        color: "#CCF0F0F0"
        border.color: "#40000000"
        border.width: 1
        y: parent.height - 4
        visible: !root.expanded
    }
}