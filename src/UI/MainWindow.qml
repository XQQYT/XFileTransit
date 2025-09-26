import QtQuick 2.14
import QtQuick.Window 2.14
import Qt.labs.platform 1.1

Window {
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

    // 主窗口的拖拽区域
    DropArea {
        anchors.fill: parent
        onEntered: {
            console.log("主窗口: 拖拽进入, hasUrls:", drag.hasUrls, "formats:", drag.formats)
            if (drag.hasUrls) {
                drag.accept()
                dragActive = true
                if (!root.expanded) {
                    root.expanded = true
                }
            }
        }
        onExited: {
            console.log("主窗口: 拖拽退出")
            dragActive = false
        }
        onDropped: function(drop) {
            console.log("主窗口: 文件拖放, hasUrls:", drop.hasUrls, "urls count:", drop.urls ? drop.urls.length : 0)
            dragActive = false
            if (drop.hasUrls && drop.urls) {
                for (var i = 0; i < drop.urls.length; i++) {
                    var fileUrl = drop.urls[i].toString()
                    console.log("主窗口文件", i + 1, ":", fileUrl)
                    // 转换为本地路径
                    if (fileUrl.startsWith("file:///")) {
                        var filePath = fileUrl.substring(8) // 移除 file:///
                        console.log("本地路径:", filePath)
                    }
                }
                drop.accept()
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
            if (!isMouseInTriggerArea(mouseX, mouseY)) {
                collapseTimer.start()
            }
        }

        function isMouseInTriggerArea(x, y) {
            var globalPos = mapToItem(null, x, y)
            return globalPos.y >= 0 && globalPos.y <= 6 &&
                    globalPos.x >= root.x && globalPos.x <= (root.x + root.width)
        }
    }

    // 延迟收缩
    Timer {
        id: collapseTimer
        interval: 150
        onTriggered: {
            if (root.expanded) {
                root.expanded = false
            }
        }
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
                collapseTimer.stop()
                if (!root.expanded) {
                    root.expanded = true
                }
            }
            onExited: {
                var globalPos = mapToItem(null, mouseX, mouseY)
                var inMainWindow = globalPos.y >= root.y && globalPos.y <= (root.y + root.height) &&
                        globalPos.x >= root.x && globalPos.x <= (root.x + root.width)
                if (!inMainWindow) {
                    collapseTimer.start()
                }
            }
        }

        // 拖拽进入时展开
        DropArea {
            anchors.fill: parent
            onEntered: {
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
                    for (var i = 0; i < drop.urls.length; i++) {
                        var fileUrl = drop.urls[i].toString()
                        console.log("触发窗口文件", i + 1, ":", fileUrl)
                        // 转换为本地路径
                        if (fileUrl.startsWith("file:///")) {
                            var filePath = fileUrl.substring(8)
                            console.log("本地路径:", filePath)
                        }
                    }
                    drop.accept()
                } else {
                    console.log("触发窗口: 没有检测到文件URL")
                }
                collapseTimer.start()
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
        } else {
            root.y = -root.height + 4
        }
    }

    // 背景
    Rectangle {
        anchors.fill: parent
        radius: 20
        color: dragActive ? "#88A8DFF7" : "#CCF0F0F0"
        border.color: "#40000000"
        border.width: 1
    }

    // 内容
    Text {
        text: dragActive ? "🔄 释放文件以处理" : "🔄 文件中转站 - 拖放文件到此"
        font.pixelSize: 14
        color: "#2c3e50"
        anchors.centerIn: parent
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
        }
    }
}
