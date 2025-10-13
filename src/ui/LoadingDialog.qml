import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Popup {
    id: loadingDialog
    width: 200
    height: 160
    modal: true
    dim: true
    closePolicy: Popup.NoAutoClose
    
    // 公共接口
    property string message: qsTr("加载中...")
    property string buttonText: qsTr("取消")
    
    property bool autoCenter: true
    
    onOpened: {
        if (autoCenter && parent) {
            x = (parent.width - width) / 2
            y = (parent.height - height) / 2
        }
    }

    signal buttonClicked()
    
    background: Rectangle {
        color: "white"
        radius: 12
        border.color: "#e0e0e0"
        border.width: 1
    }
    
    contentItem: ColumnLayout {
        spacing: 16
        
        BusyIndicator {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 40
            Layout.preferredHeight: 40
        }
        
        Label {
            text: loadingDialog.message
            font.pixelSize: 14
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
        }
        
        Button {
            text: loadingDialog.buttonText
            Layout.alignment: Qt.AlignHCenter
            
            // 直接使用默认样式，避免自定义导致的递归
            onClicked: {
                loadingDialog.buttonClicked()
                loadingDialog.close()
            }
        }
    }
    
    function show(msg, btnText) {
        if (msg !== undefined) message = msg
        if (btnText !== undefined) buttonText = btnText
        open()
    }
    
    function hide() {
        close()
    }
}