import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow  {
    id: root
    width: 350
    height: 150
    modality: Qt.ApplicationModal
    flags: Qt.Dialog
    
    property string title: "提示"
    property string text: ""
    property int buttons: root.ok
    property int iconType: root.none  // 默认信息图标
    
    // 按钮类型枚举
    readonly property int ok: 0x0001
    readonly property int cancel: 0x0002
    readonly property int yes: 0x0004
    readonly property int no: 0x0008
    
    // 图标类型枚举
    readonly property int none: 0      // 无图标
    readonly property int info: 1      // 信息图标
    readonly property int success: 2   // 成功图标
    readonly property int error: 3     // 错误图标
    readonly property int warning: 4   // 警告图标
    
    // 信号
    signal accepted()
    signal rejected()
    signal clicked(int button)
    
    onVisibleChanged: {
        if (visible) {
            requestActivate() // 请求激活窗口
            forceActiveFocus() // 强制获得焦点
        }
    }

    Rectangle {
        id: bgRect
        anchors.fill: parent
        radius: 8
        color: "#ffffff"
    }
    
    RowLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15
        
        // 图标区域
        Rectangle {
            id: iconContainer
            Layout.preferredWidth: 40
            Layout.preferredHeight: 40
            Layout.alignment: Qt.AlignTop
            radius: 20
            color: getIconColor()
            visible: root.iconType !== root.none
            
            Text {
                anchors.centerIn: parent
                text: getIconText()
                font.pixelSize: 18
                font.family: "Segoe UI Symbol"
                color: "white"
                font.bold: true
            }
        }
        
        // 内容和按钮区域
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 15
            
            // 标题
            Text {
                text: root.title
                font.bold: true
                font.pixelSize: 16
                color: "#333333"
                Layout.fillWidth: true
            }
            
            // 消息内容
            Text {
                text: root.text
                font.pixelSize: 14
                color: "#666666"
                wrapMode: Text.Wrap
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
            
            // 按钮区域
            Row  {
                Layout.alignment: Qt.AlignRight
                spacing: 10
                
                Button {
                    width: 60
                    text: "是"
                    visible: (root.buttons & root.yes) && !(root.buttons & root.ok)
                    onClicked: {
                        root.clicked(root.yes)
                        root.accepted()
                        root.hide()
                    }
                }
                
                Button {
                    width: 60
                    text: "否"
                    visible: (root.buttons & root.no)
                    onClicked: {
                        root.clicked(root.no)
                        root.rejected()
                        root.hide()
                    }
                }
                
                Button {
                    width: 60
                    text: "确定"
                    visible: (root.buttons & root.ok) && !(root.buttons & root.yes)
                    onClicked: {
                        root.clicked(root.ok)
                        root.accepted()
                        root.hide()
                    }
                }
                
                Button {
                    width: 60
                    text: "取消"
                    visible: (root.buttons & root.cancel)
                    flat: true
                    onClicked: {
                        root.clicked(root.cancel)
                        root.rejected()
                        root.hide()
                    }
                }
            }
        }
    }
    
    // 获取图标颜色
    function getIconColor() {
        switch(root.iconType) {
            case root.success: return "#4CAF50"  // 绿色
            case root.error: return "#F44336"    // 红色
            case root.warning: return "#FF9800"  // 橙色
            case root.info: return "#2196F3"     // 蓝色
            default: return "transparent"
        }
    }
    
    // 获取图标字符（使用 Unicode 字符）
    function getIconText() {
        switch(root.iconType) {
            case root.success: return "✓"  // 对勾
            case root.error: return "✕"    // 叉号
            case root.warning: return "!"  // 感叹号
            case root.info: return "i"     // 字母 i
            default: return ""
        }
    }
}