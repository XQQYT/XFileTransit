import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Popup {
    id: root
    width: 300
    height: 150
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    
    property string title: "提示"
    property string text: ""
    property int buttons: root.ok
    
    // 按钮类型枚举
    readonly property int ok: 0x0001
    readonly property int cancel: 0x0002
    readonly property int yes: 0x0004
    readonly property int no: 0x0008
    
    // 信号
    signal accepted()
    signal rejected()
    signal clicked(int button)
    
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    
    Rectangle {
        id: bgRect
        anchors.fill: parent
        radius: 8
        color: "#ffffff"
    }
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
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
        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: 10
            
            Button {
                text: "是"
                visible: (root.buttons & root.yes) && !(root.buttons & root.ok)
                onClicked: {
                    root.clicked(root.yes)
                    root.accepted()
                    root.close()
                }
            }
            
            Button {
                text: "否"
                visible: (root.buttons & root.no)
                onClicked: {
                    root.clicked(root.no)
                    root.rejected()
                    root.close()
                }
            }
            
            Button {
                text: "确定"
                visible: (root.buttons & root.ok) && !(root.buttons & root.yes)
                onClicked: {
                    root.clicked(root.ok)
                    root.accepted()
                    root.close()
                }
            }
            
            Button {
                text: "取消"
                visible: (root.buttons & root.cancel)
                flat: true
                onClicked: {
                    root.clicked(root.cancel)
                    root.rejected()
                    root.close()
                }
            }
        }
    }
}