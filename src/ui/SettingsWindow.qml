import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.platform

ApplicationWindow {
    id: window
    width: 1000
    height: 650
    minimumWidth: 850
    minimumHeight: 550
    title: qsTr("设置")
    visible: false
    
    property color primaryColor: "#6366f1"
    property color primaryLightColor: "#a5b4fc"
    property color backgroundColor: "#f8fafc"
    property color sidebarColor: "#ffffff"
    property color cardColor: "#ffffff"
    property color textPrimary: "#1e293b"
    property color textSecondary: "#64748b"
    property color borderColor: "#e2e8f0"
    property color dividerColor: "#f1f5f9"
    
    property color accentGreen: "#10b981"
    property color accentRed: "#dc2626"
    property color accentOrange: "#f59e0b"
    
    property color switchOffColor: "#e2e8f0"
    property color switchHandleColor: "white"
    property color clearCacheButtonBg: "#fee2e2"
    property color clearCacheButtonBorder: "#fecaca"
    property color sliderTrackColor: "#e2e8f0"
    property color progressBarBg: "#e2e8f0"
    property color updateInfoBg: "#f8fafc"
    property color whiteColor: "white"
    
    property var settings_model: null
    
    property var currentBtn: null
    property var currentPage: basicSettingsPage  // 当前页面组件

    property string usedSize: "---"
    property string freeSize: "---"
    property string totalSize: "---"
    
    property bool isDragging: false
    property int dragStartX: 0
    property int dragStartY: 0
    
    // 主题切换处理
    function switchTheme(theme) {
        settings_model.currentTheme = theme
        setTheme(theme)
        console.log("切换主题:", theme)
    }
    
    // 语言切换处理
    function switchLanguage(language) {
        settings_model.currentLanguage = language
        console.log("切换语言:", language)
    }
    
    // 选择缓存目录
    function chooseCachePath() {
        console.log("选择缓存目录")
        folderDialog.open()
    }
    
    // 切换自动下载
    function toggleAutoDownload(enabled) {
        settings_model.autoDownload = enabled
        console.log("自动下载:", enabled ? "启用" : "禁用")
    }
    
    // 设置并发传输数
    function setConcurrentTransfers(count) {
        settings_model.concurrentTransfers = count
        console.log("设置并发传输数:", count)
    }
    
    // 切换加密
    function toggleEncryption(enabled) {
        settings_model.enableEncryption = enabled
        console.log("传输加密:", enabled ? "启用" : "禁用")
    }
    
    // 切换智能展开
    function toggleExpandOnAction(enabled) {
        settings_model.expandOnAction = enabled
        console.log("智能展开:", enabled ? "启用" : "禁用")
    }
    
    // 检查更新
    function checkForUpdates() {
        settings_model.isUpdateAvailable = true
        console.log("检查更新")
    }
    
    // 处理鼠标拖动
    function handleMousePress(mouse) {
        isDragging = true
        dragStartX = mouse.x
        dragStartY = mouse.y
    }
    
    function handleMouseMove(mouse) {
        if (isDragging) {
        }
    }
    
    function handleMouseRelease(mouse) {
        isDragging = false
    }
    
    // 居中显示
    function centerOnScreen() {
        Qt.callLater(function() {
            var screenWidth = Screen.width > 0 ? Screen.width : 1920
            var screenHeight = Screen.height > 0 ? Screen.height : 1080
            
            window.x = Math.max(0, (screenWidth - window.width) / 2)
            window.y = Math.max(0, (screenHeight - window.height) / 2)
        })
    }
    
    onVisibleChanged: {
        if (visible) {
            centerOnScreen()
            requestActivate()
        }
    }
    

    // 主题切换函数
    function setTheme(theme_index) {
        switch (theme_index) 
        {
            case 0:
                // 浅色主题
                primaryColor = "#6366f1"
                primaryLightColor = "#a5b4fc"
                backgroundColor = "#f8fafc"
                sidebarColor = "#ffffff"
                cardColor = "#ffffff"
                textPrimary = "#1e293b"
                textSecondary = "#64748b"
                borderColor = "#e2e8f0"
                dividerColor = "#f1f5f9"
                accentGreen = "#10b981"
                accentRed = "#dc2626"
                accentOrange = "#f59e0b"
                switchOffColor = "#e2e8f0"
                clearCacheButtonBg = "#fee2e2"
                clearCacheButtonBorder = "#fecaca"
                sliderTrackColor = "#e2e8f0"
                progressBarBg = "#e2e8f0"
                updateInfoBg = "#f8fafc"
                break
            case 1:
                // 深色主题
                primaryColor = "#6366f1"
                primaryLightColor = "#4f46e5"
                backgroundColor = "#0f172a"
                sidebarColor = "#1e293b"
                cardColor = "#1e293b"
                textPrimary = "#f8fafc"
                textSecondary = "#cbd5e1"
                borderColor = "#334155"
                dividerColor = "#334155"
                accentGreen = "#10b981"
                accentRed = "#dc2626"
                accentOrange = "#f59e0b"
                switchOffColor = "#4b5563"
                clearCacheButtonBg = "#7f1d1d"
                clearCacheButtonBorder = "#991b1b"
                sliderTrackColor = "#4b5563"
                progressBarBg = "#4b5563"
                updateInfoBg = "#1f2937"
                break
            default:
                return
        }
        
        // 保存当前组件
        var currentComp = currentPage
        
        // 强制重新加载当前页面
        currentPage = null
        Qt.callLater(function() {
            currentPage = currentComp
        })
        
        // 强制重绘当前选中按钮
        if (currentBtn) {
            currentBtn.color = primaryColor
        }
    }

    LoadingDialog {
        id: load_dialog
        enableBtn: false
    }
    FolderDialog {
        id: folderDialog
        title: "选择目录"
        options: FolderDialog.ShowDirsOnly

        onAccepted: {
            settings_model.cachePath = folder
            load_dialog.show(qsTr("迁移中"), qsTr("取消"))
        }
    }

    // 导航项组件
    Component {
        id: navItemComponent
        
        Rectangle {
            id: navDelegate
            property string itemTitleKey: ""
            property string itemSubtitleKey: ""

            property string itemTitle: itemTitleKey ? qsTr(itemTitleKey) : ""
            property string itemSubtitle: itemSubtitleKey ? qsTr(itemSubtitleKey) : ""
            property var pageComponent: null  // 对应的页面组件
            
            width: parent.width
            height: 60
            radius: 12
            color: currentBtn === navDelegate ? primaryColor : "transparent"
            
            Row {
                anchors.fill: parent
                anchors.leftMargin: 15
                spacing: 12
                
                
                Column {
                    spacing: 2
                    anchors.verticalCenter: parent.verticalCenter
                    
                    Text {
                        text: itemTitle
                        font.pixelSize: 16
                        font.weight: currentBtn === navDelegate ? Font.Bold : Font.Normal
                        color: currentBtn === navDelegate ? whiteColor : textPrimary
                    }
                    
                    Text {
                        text: itemSubtitle
                        font.pixelSize: 11
                        color: currentBtn === navDelegate ? "rgba(255,255,255,0.8)" : textSecondary
                    }
                }
            }
            
            Rectangle {
                visible: currentBtn === navDelegate
                width: 4
                height: 20
                radius: 2
                color: whiteColor
                anchors {
                    right: parent.right
                    rightMargin: 10
                    verticalCenter: parent.verticalCenter
                }
            }
            
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                hoverEnabled: true
                onClicked: {
                    if (currentBtn) {
                        currentBtn.color = "transparent"
                    }
                    navDelegate.color = primaryColor
                    currentBtn = navDelegate
                    currentPage = pageComponent
                }
                
                onEntered: {
                    if (currentBtn !== navDelegate) {
                        navDelegate.color = dividerColor
                    }
                }
                onExited: {
                    if (currentBtn !== navDelegate) {
                        navDelegate.color = "transparent"
                    }
                }
            }
        }
    }
    
    Rectangle {
        anchors.fill: parent
        color: backgroundColor
        
        // 左侧导航栏
        Rectangle {
            id: sidebar
            width: 260
            anchors {
                top: parent.top
                bottom: parent.bottom
                left: parent.left
            }
            color: sidebarColor
            
            Rectangle {
                width: 4
                height: parent.height
                color: primaryColor
            }
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 0
                
                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 80
                    
                    Row {
                        spacing: 12
                        anchors.centerIn: parent
                        
                        Rectangle {
                            width: 40
                            height: 40
                            radius: 10
                            color: primaryColor
                            
                            Text {
                                text: "⚙️"
                                font.pixelSize: 20
                                anchors.centerIn: parent
                            }
                        }
                        
                        Column {
                            spacing: 2
                            
                            Text {
                                text: "SETTINGS"
                                font {
                                    pixelSize: 18
                                    weight: Font.Bold
                                    letterSpacing: 1
                                }
                                color: primaryColor
                            }
                            
                            Text {
                                text: qsTr("控制中心")
                                font.pixelSize: 12
                                color: textSecondary
                            }
                        }
                    }
                }
                
                Column {
                    id: navColumn
                    Layout.fillWidth: true
                    Layout.topMargin: 20
                    spacing: 8
                    
                    // 基础设置导航项
                    Loader {
                        id: basicNavItem
                        width: parent.width
                        sourceComponent: navItemComponent
                        
                        onLoaded: {
                            item.itemTitleKey = "基础设置"
                            item.itemSubtitleKey = "Basic Settings"
                            item.pageComponent = basicSettingsPage
                        }
                        
                        Component.onCompleted: {
                            if (currentBtn === null) {
                                currentBtn = item
                            }
                        }
                    }
                    
                    // 文件设置导航项
                    Loader {
                        id: fileNavItem
                        width: parent.width
                        sourceComponent: navItemComponent
                        
                        onLoaded: {
                            item.itemTitleKey = "文件设置"
                            item.itemSubtitleKey = "File Settings"
                            item.pageComponent = fileSettingsPage
                        }
                    }
                    
                    // 传输设置导航项
                    Loader {
                        id: transferNavItem
                        width: parent.width
                        sourceComponent: navItemComponent
                        
                        onLoaded: {
                            item.itemTitleKey = "传输设置"
                            item.itemSubtitleKey = "Transfer Settings"
                            item.pageComponent = transferSettingsPage
                        }
                    }
                    
                    // 通知与提醒导航项
                    Loader {
                        id: notificationNavItem
                        width: parent.width
                        sourceComponent: navItemComponent
                        
                        onLoaded: {
                            item.itemTitleKey = "通知与提醒"
                            item.itemSubtitleKey = "Notifications"
                            item.pageComponent = notificationSettingsPage
                        }
                    }
                    
                    // 关于软件导航项
                    Loader {
                        id: aboutNavItem
                        width: parent.width
                        sourceComponent: navItemComponent
                        
                        onLoaded: {
                            item.itemTitleKey = "关于软件"
                            item.itemSubtitleKey = "About"
                            item.pageComponent = aboutSettingsPage
                        }
                    }
                }
                
                Item { Layout.fillHeight: true }
                
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 60
                    radius: 12
                    color: dividerColor
                    
                    Row {
                        anchors.centerIn: parent
                        spacing: 10
                        
                        Rectangle {
                            width: 36
                            height: 36
                            radius: 18
                            color: primaryColor
                            
                            Text {
                                text: "U"
                                font {
                                    pixelSize: 16
                                    weight: Font.Bold
                                }
                                color: whiteColor
                                anchors.centerIn: parent
                            }
                        }
                        
                        Column {
                            spacing: 2
                            anchors.verticalCenter: parent.verticalCenter
                            
                            Text {
                                text: qsTr("用户设置")
                                font.pixelSize: 14
                                color: textPrimary
                            }
                            
                            Text {
                                text: settings_model.appVersion
                                font.pixelSize: 11
                                color: textSecondary
                            }
                        }
                    }
                }
            }
        }
        
        Rectangle {
            width: 1
            anchors {
                top: parent.top
                bottom: parent.bottom
                left: sidebar.right
            }
            color: borderColor
        }
        
        // 显示当前页面
        Loader {
            id: contentLoader
            anchors {
                top: parent.top
                bottom: parent.bottom
                left: sidebar.right
                right: parent.right
            }
            sourceComponent: currentPage
        }
    }
    
    // 基础设置页面
    Component {
        id: basicSettingsPage
        
        Item {
            anchors.fill: parent
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 30
                spacing: 30
                
                Column {
                    spacing: 8
                    
                    Text {
                        text: qsTr("基础设置")
                        font {
                            pixelSize: 28
                            weight: Font.Bold
                        }
                        color: textPrimary
                    }
                    
                    Text {
                        text: "Basic Settings"
                        font.pixelSize: 16
                        color: textSecondary
                    }
                }
                
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    
                    contentWidth: parent.width

                    Column {
                        id: contentColumn
                        width: parent.width
                        spacing: 20
                        
                        // 主题切换卡片
                        Rectangle {
                            width: contentColumn.width
                            height: 240
                            radius: 16
                            color: cardColor
                            border.color: borderColor
                            border.width: 2
                            
                            Column {
                                anchors.fill: parent
                                anchors.margins: 20
                                spacing: 15
                                
                                Row {
                                    width: parent.width
                                    spacing: 12
                                    
                                    Column {
                                        spacing: 2
                                        anchors.verticalCenter: parent.verticalCenter
                                        
                                        Text {
                                            text: qsTr("主题切换")
                                            font {
                                                pixelSize: 18
                                                weight: Font.Bold
                                            }
                                            color: textPrimary
                                        }
                                        
                                        Text {
                                            text: qsTr("切换应用的主题模式")
                                            font.pixelSize: 13
                                            color: textSecondary
                                        }
                                    }
                                }
                                
                                Row {
                                    spacing: 15
                                    width: parent.width
                                    // 浅色主题选项
                                    Rectangle {
                                        id: lightTheme
                                        width: parent.width / 3 - 15
                                        height: 120
                                        radius: 12
                                        color: settings_model.currentTheme === 0 ? primaryColor : cardColor
                                        border.color: settings_model.currentTheme === 0 ? primaryLightColor : borderColor
                                        border.width: settings_model.currentTheme === 0 ? 3 : 2
                                        
                                        Column {
                                            anchors.centerIn: parent
                                            spacing: 8
                                            
                                            Text {
                                                text: "☀️"
                                                font.pixelSize: 30
                                                anchors.horizontalCenter: parent.horizontalCenter
                                            }
                                            
                                            Text {
                                                text: qsTr("浅色")
                                                font.pixelSize: 14
                                                font.weight: Font.Medium
                                                color: settings_model.currentTheme === 0 ? whiteColor : textPrimary
                                                anchors.horizontalCenter: parent.horizontalCenter
                                            }
                                            
                                            Text {
                                                text: "Light"
                                                font.pixelSize: 11
                                                color: settings_model.currentTheme === 0 ? "rgba(255,255,255,0.8)" : textSecondary
                                                anchors.horizontalCenter: parent.horizontalCenter
                                            }
                                        }
                                        
                                        MouseArea {
                                            anchors.fill: parent
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: switchTheme(0)
                                        }
                                    }
                                    
                                    // 深色主题选项
                                    Rectangle {
                                        id: darkTheme
                                        width: parent.width / 3 - 15
                                        height: 120
                                        radius: 12
                                        color: settings_model.currentTheme === 1 ? primaryColor : cardColor
                                        border.color: settings_model.currentTheme === 1 ? primaryLightColor : borderColor
                                        border.width: settings_model.currentTheme === 1 ? 3 : 2
                                        
                                        Column {
                                            anchors.centerIn: parent
                                            spacing: 8
                                            
                                            Text {
                                                text: "🌙"
                                                font.pixelSize: 30
                                                anchors.horizontalCenter: parent.horizontalCenter
                                            }
                                            
                                            Text {
                                                text: qsTr("深色")
                                                font.pixelSize: 14
                                                font.weight: Font.Medium
                                                color: settings_model.currentTheme === 1 ? whiteColor : textPrimary
                                                anchors.horizontalCenter: parent.horizontalCenter
                                            }
                                            
                                            Text {
                                                text: "Dark"
                                                font.pixelSize: 11
                                                color: settings_model.currentTheme === 1 ? "rgba(255,255,255,0.8)" : textSecondary
                                                anchors.horizontalCenter: parent.horizontalCenter
                                            }
                                        }
                                        
                                        MouseArea {
                                            anchors.fill: parent
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: switchTheme(1)
                                        }
                                    }
                                    
                                    // 自动主题选项
                                    Rectangle {
                                        id: autoTheme
                                        width: parent.width / 3 - 15
                                        height: 120
                                        radius: 12
                                        color: settings_model.currentTheme === 2 ? primaryColor : cardColor
                                        border.color: settings_model.currentTheme === 2 ? primaryLightColor : borderColor
                                        border.width: settings_model.currentTheme === 2 ? 3 : 2
                                        
                                        Column {
                                            anchors.centerIn: parent
                                            spacing: 8
                                            
                                            
                                            Text {
                                                text: qsTr("自动")
                                                font.pixelSize: 14
                                                font.weight: Font.Medium
                                                color: settings_model.currentTheme === 2 ? whiteColor : textPrimary
                                                anchors.horizontalCenter: parent.horizontalCenter
                                            }
                                            
                                            Text {
                                                text: "Auto"
                                                font.pixelSize: 11
                                                color: settings_model.currentTheme === 2 ? "rgba(255,255,255,0.8)" : textSecondary
                                                anchors.horizontalCenter: parent.horizontalCenter
                                            }
                                        }
                                        
                                        MouseArea {
                                            anchors.fill: parent
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: switchTheme(2)
                                        }
                                    }
                                }
                            }
                        }
                        
                        // 语言切换卡片
                        Rectangle {
                            width: parent.width
                            height: 200
                            radius: 16
                            color: cardColor
                            border.color: borderColor
                            border.width: 2
                            
                            Column {
                                anchors.fill: parent
                                anchors.margins: 20
                                spacing: 15
                                
                                Row {
                                    width: parent.width
                                    spacing: 12
                                    
                                    
                                    Column {
                                        spacing: 2
                                        anchors.verticalCenter: parent.verticalCenter
                                        
                                        Text {
                                            text: qsTr("语言切换")
                                            font {
                                                pixelSize: 18
                                                weight: Font.Bold
                                            }
                                            color: textPrimary
                                        }
                                        
                                        Text {
                                            text: qsTr("选择界面显示语言")
                                            font.pixelSize: 13
                                            color: textSecondary
                                        }
                                    }
                                }
                                
                                Rectangle {
                                    width: parent.width
                                    height: 50
                                    radius: 10
                                    color: backgroundColor
                                    border.color: borderColor
                                    border.width: 2
                                    
                                    Row {
                                        anchors.fill: parent
                                        
                                        Rectangle {
                                            id: chineseLang
                                            width: parent.width / 2
                                            height: parent.height
                                            color: settings_model.currentLanguage === 0 ? primaryColor : "transparent"
                                            radius: 10
                                            
                                            Text {
                                                text: "简体中文"
                                                font.pixelSize: 16
                                                font.weight: Font.Bold
                                                color: settings_model.currentLanguage === 0 ? whiteColor : textPrimary
                                                anchors.centerIn: parent
                                            }
                                            
                                            MouseArea {
                                                anchors.fill: parent
                                                cursorShape: Qt.PointingHandCursor
                                                onClicked: switchLanguage(0)
                                            }
                                        }
                                        
                                        Rectangle {
                                            id: englishLang
                                            width: parent.width / 2
                                            height: parent.height
                                            color: settings_model.currentLanguage === 1 ? primaryColor : "transparent"
                                            radius: 10
                                            
                                            Text {
                                                text: "English"
                                                font.pixelSize: 16
                                                font.weight: settings_model.currentLanguage === 1 ? Font.Bold : Font.Normal
                                                color: settings_model.currentLanguage === 1 ? whiteColor : textPrimary
                                                anchors.centerIn: parent
                                            }
                                            
                                            MouseArea {
                                                anchors.fill: parent
                                                cursorShape: Qt.PointingHandCursor
                                                onClicked: switchLanguage(1)
                                            }
                                        }
                                    }
                                }
                                
                                Text {
                                    text: qsTr("当前语言: ") + (settings_model.currentLanguage === 0 ? "简体中文" : "English")
                                    font.pixelSize: 14
                                    color: textSecondary
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    // 文件设置页面
    Component {
        id: fileSettingsPage
        
        Item {
            anchors.fill: parent
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 30
                spacing: 30
                
                Column {
                    spacing: 8
                    
                    Text {
                        text: qsTr("文件设置")
                        font {
                            pixelSize: 28
                            weight: Font.Bold
                        }
                        color: textPrimary
                    }
                    
                    Text {
                        text: "File Settings"
                        font.pixelSize: 16
                        color: textSecondary
                    }
                }
                
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    
                    Column {
                        width: parent.width
                        spacing: 20
                        
                        Rectangle {
                            width: parent.width
                            height: 220
                            radius: 16
                            color: cardColor
                            border.color: borderColor
                            border.width: 2
                            
                            Column {
                                anchors.fill: parent
                                anchors.margins: 20
                                spacing: 15
                                
                                Row {
                                    width: parent.width
                                    spacing: 12
                                    
                                    
                                    Column {
                                        spacing: 2
                                        anchors.verticalCenter: parent.verticalCenter
                                        
                                        Text {
                                            text: qsTr("缓存目录")
                                            font {
                                                pixelSize: 18
                                                weight: Font.Bold
                                            }
                                            color: textPrimary
                                        }
                                        
                                        Text {
                                            text: qsTr("设置缓存文件的存储位置")
                                            font.pixelSize: 13
                                            color: textSecondary
                                        }
                                    }
                                }
                                
                                Rectangle {
                                    width: parent.width
                                    height: 50
                                    radius: 10
                                    color: backgroundColor
                                    border.color: borderColor
                                    border.width: 2
                                    
                                    Row {
                                        anchors.fill: parent
                                        anchors.margins: 5
                                        
                                        Text {
                                            text: settings_model.cachePath
                                            font.pixelSize: 14
                                            color: textPrimary
                                            elide: Text.ElideMiddle
                                            anchors.verticalCenter: parent.verticalCenter
                                            width: parent.width - 100
                                        }
                                        
                                        Rectangle {
                                            width: 90
                                            height: 36
                                            radius: 8
                                            color: primaryColor
                                            anchors.verticalCenter: parent.verticalCenter
                                            
                                            Text {
                                                text: qsTr("更改")
                                                font.pixelSize: 14
                                                color: whiteColor
                                                anchors.centerIn: parent
                                            }
                                            
                                            MouseArea {
                                                anchors.fill: parent
                                                cursorShape: Qt.PointingHandCursor
                                                hoverEnabled: true
                                                onClicked: chooseCachePath()
                                                
                                                onEntered: parent.opacity = 0.8
                                                onExited: parent.opacity = 1
                                            }
                                        }
                                    }
                                }
                                
                                Row {
                                    spacing: 20
                                    
                                    Column {
                                        spacing: 4
                                        
                                        Text {
                                            text: qsTr("已使用")
                                            font.pixelSize: 12
                                            color: textSecondary
                                            anchors.horizontalCenter: parent.horizontalCenter
                                        }
                                        
                                        Text {
                                            text: usedSize
                                            font {
                                                pixelSize: 16
                                                weight: Font.Bold
                                            }
                                            color: primaryColor
                                            anchors.horizontalCenter: parent.horizontalCenter
                                        }
                                    }
                                    
                                    Column {
                                        spacing: 4
                                        
                                        Text {
                                            text: qsTr("可用")
                                            font.pixelSize: 12
                                            color: textSecondary
                                            anchors.horizontalCenter: parent.horizontalCenter
                                        }
                                        
                                        Text {
                                            text: freeSize
                                            font {
                                                pixelSize: 16
                                                weight: Font.Bold
                                            }
                                            color: accentGreen
                                            anchors.horizontalCenter: parent.horizontalCenter
                                        }
                                    }
                                    
                                    Column {
                                        spacing: 4
                                        
                                        Text {
                                            text: qsTr("总大小")
                                            font.pixelSize: 12
                                            color: textSecondary
                                            anchors.horizontalCenter: parent.horizontalCenter
                                        }
                                        
                                        Text {
                                            text: totalSize
                                            font {
                                                pixelSize: 16
                                                weight: Font.Bold
                                            }
                                            color: textSecondary
                                            anchors.horizontalCenter: parent.horizontalCenter
                                        }
                                    }

                                    Connections{
                                        target: settings_model
                                        function onCacheInfoDone(used,free,total) {
                                            usedSize = used
                                            freeSize = free
                                            totalSize = total
                                        }
                                    }

                                    Connections{
                                        target: settings_model
                                        function onCacheMoveDone() {
                                            Qt.callLater(function() {
                                                load_dialog.close()
                                            })
                                        }
                                    }  
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    // 传输设置页面
    Component {
        id: transferSettingsPage
        
        Item {
            anchors.fill: parent
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 30
                spacing: 30
                
                Column {
                    spacing: 8
                    
                    Text {
                        text: qsTr("传输设置")
                        font {
                            pixelSize: 28
                            weight: Font.Bold
                        }
                        color: textPrimary
                    }
                    
                    Text {
                        text: "Transfer Settings"
                        font.pixelSize: 16
                        color: textSecondary
                    }
                }
                
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    
                    Column {
                        width: parent.width
                        spacing: 20
                        
                        Rectangle {
                            width: parent.width
                            height: 140
                            radius: 16
                            color: cardColor
                            border.color: borderColor
                            border.width: 2
                            
                            Column {
                                anchors.fill: parent
                                anchors.margins: 20
                                spacing: 15
                                
                                Row {
                                    width: parent.width
                                    spacing: 12
                                    
                                    
                                    Column {
                                        spacing: 2
                                        anchors.verticalCenter: parent.verticalCenter
                                        
                                        Text {
                                            text: qsTr("文件自动下载")
                                            font {
                                                pixelSize: 18
                                                weight: Font.Bold
                                            }
                                            color: textPrimary
                                        }
                                        
                                        Text {
                                            text: qsTr("小于50MB的文件自动开始下载")
                                            font.pixelSize: 13
                                            color: textSecondary
                                        }
                                    }
                                }
                                
                                Row {
                                    spacing: 20
                                    width: parent.width
                                    
                                    Text {
                                        text: qsTr("启用自动下载")
                                        font.pixelSize: 16
                                        color: textPrimary
                                        anchors.verticalCenter: parent.verticalCenter
                                    }
                                    
                                    Item { width: 20; height: 1 }
                                    
                                    Rectangle {
                                        id: autoDownloadSwitch
                                        width: 60
                                        height: 30
                                        radius: 15
                                        color: settings_model.autoDownload ? primaryColor : switchOffColor
                                        anchors.verticalCenter: parent.verticalCenter
                                        
                                        Rectangle {
                                            x: settings_model.autoDownload ? parent.width - width - 3 : 3
                                            y: 3
                                            width: 24
                                            height: 24
                                            radius: 12
                                            color: switchHandleColor
                                            
                                            Behavior on x {
                                                NumberAnimation { duration: 200 }
                                            }
                                        }
                                        
                                        MouseArea {
                                            anchors.fill: parent
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: toggleAutoDownload(!settings_model.autoDownload)
                                        }
                                    }
                                }
                            }
                        }
                        
                        Rectangle {
                            width: parent.width
                            height: 150
                            radius: 16
                            color: cardColor
                            border.color: borderColor
                            border.width: 2
                            
                            Column {
                                anchors.fill: parent
                                anchors.margins: 20
                                spacing: 20
                                
                                Row {
                                    width: parent.width
                                    spacing: 12
                                    

                                    Column {
                                        spacing: 2
                                        anchors.verticalCenter: parent.verticalCenter
                                        
                                        Text {
                                            text: qsTr("同时传输任务数")
                                            font {
                                                pixelSize: 18
                                                weight: Font.Bold
                                            }
                                            color: textPrimary
                                        }
                                        
                                        Text {
                                            text: qsTr("设置最大并行传输任务数量")
                                            font.pixelSize: 13
                                            color: textSecondary
                                        }
                                    }
                                }
                                
                                Row {
                                    spacing: 20
                                    width: parent.width
                                    
                                    Text {
                                        text: qsTr("最大任务数:")
                                        font.pixelSize: 16
                                        color: textPrimary
                                        anchors.verticalCenter: parent.verticalCenter
                                    }
                                    
                                    Text {
                                        id: valueText
                                        text: settings_model.concurrentTransfers
                                        font {
                                            pixelSize: 24
                                            weight: Font.Bold
                                        }
                                        color: primaryColor
                                        anchors.verticalCenter: parent.verticalCenter
                                        width: 30
                                        horizontalAlignment: Text.AlignRight
                                    }
                                    
                                    Item { width: 20; height: 1 }
                                    
                                    Rectangle {
                                        id: sliderTrack
                                        width: 200
                                        height: 8
                                        radius: 4
                                        color: sliderTrackColor
                                        anchors.verticalCenter: parent.verticalCenter
                                        
                                        Rectangle {
                                            id: sliderFill
                                            width: (settings_model.concurrentTransfers - 1) / 9 * parent.width
                                            height: parent.height
                                            radius: 4
                                            color: primaryColor
                                        }
                                        
                                        Rectangle {
                                            id: sliderHandle
                                            width: 20
                                            height: 20
                                            radius: 10
                                            color: primaryColor
                                            border.color: whiteColor
                                            border.width: 2
                                            anchors.verticalCenter: parent.verticalCenter
                                            x: sliderFill.width - width/2
                                            
                                        }
                                        
                                        MouseArea {
                                            anchors.fill: parent
                                            cursorShape: Qt.PointingHandCursor
                                            
                                            property bool isDragging: false
                                            
                                            function updateValue(mouseX) {
                                                var pos = Math.max(0, Math.min(mouseX, sliderTrack.width))
                                                var ratio = pos / sliderTrack.width
                                                var newValue = Math.round(ratio * 9) + 1
                                                setConcurrentTransfers(newValue)
                                            }
                                            
                                            onPressed: function(mouse) {
                                                isDragging = true
                                                updateValue(mouse.x)
                                            }
                                            
                                            onPositionChanged: function(mouse) {
                                                if (isDragging) {
                                                    updateValue(mouse.x)
                                                }
                                            }
                                            
                                            onReleased: function() {
                                                isDragging = false
                                            }
                                            
                                            onClicked: function(mouse) {
                                                updateValue(mouse.x)
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        
                        Rectangle {
                            width: parent.width
                            height: 160
                            radius: 16
                            color: cardColor
                            border.color: borderColor
                            border.width: 2
                            
                            Column {
                                anchors.fill: parent
                                anchors.margins: 20
                                spacing: 15
                                
                                Row {
                                    width: parent.width
                                    spacing: 12
                                    
                                    
                                    Column {
                                        spacing: 2
                                        anchors.verticalCenter: parent.verticalCenter
                                        
                                        Text {
                                            text: qsTr("是否开启加密")
                                            font {
                                                pixelSize: 18
                                                weight: Font.Bold
                                            }
                                            color: textPrimary
                                        }
                                        
                                        Text {
                                            text: qsTr("传输过程中对文件进行加密保护")
                                            font.pixelSize: 13
                                            color: textSecondary
                                        }
                                    }
                                }
                                
                                Row {
                                    spacing: 20
                                    width: parent.width
                                    
                                    Column {
                                        spacing: 5
                                        width: parent.width - 100
                                        
                                        Text {
                                            text: qsTr("启用传输加密")
                                            font.pixelSize: 16
                                            color: textPrimary
                                        }
                                        
                                        Text {
                                            text: qsTr("增强安全性，保护隐私")
                                            font.pixelSize: 13
                                            color: accentGreen
                                        }
                                    }
                                    
                                    Rectangle {
                                        id: encryptionSwitch
                                        width: 60
                                        height: 30
                                        radius: 15
                                        color: settings_model.enableEncryption ? primaryColor : switchOffColor
                                        anchors.verticalCenter: parent.verticalCenter
                                        
                                        Rectangle {
                                            x: settings_model.enableEncryption ? parent.width - width - 3 : 3
                                            y: 3
                                            width: 24
                                            height: 24
                                            radius: 12
                                            color: switchHandleColor
                                            
                                            Behavior on x {
                                                NumberAnimation { duration: 200 }
                                            }
                                        }
                                        
                                        MouseArea {
                                            anchors.fill: parent
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: toggleEncryption(!settings_model.enableEncryption)
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    // 通知与提醒页面
    Component {
        id: notificationSettingsPage
        
        Item {
            anchors.fill: parent
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 30
                spacing: 30
                
                Column {
                    spacing: 8
                    
                    Text {
                        text: qsTr("通知与提醒")
                        font {
                            pixelSize: 28
                            weight: Font.Bold
                        }
                        color: textPrimary
                    }
                    
                    Text {
                        text: "Notifications"
                        font.pixelSize: 16
                        color: textSecondary
                    }
                }
                
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    
                    Column {
                        width: parent.width
                        spacing: 20
                        
                        Rectangle {
                            width: parent.width
                            height: 140
                            radius: 16
                            color: cardColor
                            border.color: borderColor
                            border.width: 2
                            
                            Column {
                                anchors.fill: parent
                                anchors.margins: 20
                                spacing: 20
                                
                                Row {
                                    width: parent.width
                                    spacing: 12
                                    
                                    
                                    Column {
                                        spacing: 2
                                        anchors.verticalCenter: parent.verticalCenter
                                        
                                        Text {
                                            text: qsTr("有动作时展开")
                                            font {
                                                pixelSize: 18
                                                weight: Font.Bold
                                            }
                                            color: textPrimary
                                        }
                                        
                                        Text {
                                            text: qsTr("检测到活动时自动显示通知")
                                            font.pixelSize: 13
                                            color: textSecondary
                                        }
                                    }
                                }
                                
                                Row {
                                    spacing: 20
                                    
                                    Rectangle {
                                        id: expandSwitch
                                        width: 60
                                        height: 30
                                        radius: 15
                                        color: settings_model.expandOnAction ? primaryColor : switchOffColor
                                        anchors.verticalCenter: parent.verticalCenter
                                        
                                        Rectangle {
                                            x: settings_model.expandOnAction ? parent.width - width - 3 : 3
                                            y: 3
                                            width: 24
                                            height: 24
                                            radius: 12
                                            color: switchHandleColor
                                            
                                            Behavior on x {
                                                NumberAnimation { duration: 200 }
                                            }
                                        }
                                        
                                        MouseArea {
                                            anchors.fill: parent
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: toggleExpandOnAction(!settings_model.expandOnAction)
                                        }
                                    }
                                    
                                    Text {
                                        text: qsTr("启用智能展开")
                                        font.pixelSize: 16
                                        color: textPrimary
                                        anchors.verticalCenter: parent.verticalCenter
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    // 关于软件页面
    Component {
        id: aboutSettingsPage
        
        Item {
            anchors.fill: parent
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 30
                spacing: 30
                
                Column {
                    spacing: 8
                    
                    Text {
                        text: qsTr("关于软件")
                        font {
                            pixelSize: 28
                            weight: Font.Bold
                        }
                        color: textPrimary
                    }
                    
                    Text {
                        text: "About"
                        font.pixelSize: 16
                        color: textSecondary
                    }
                }
                
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    
                    Column {
                        width: parent.width
                        spacing: 20
                        
                        Rectangle {
                            width: parent.width
                            height: 180
                            radius: 16
                            color: cardColor
                            border.color: borderColor
                            border.width: 2
                            
                            Row {
                                anchors.centerIn: parent
                                spacing: 30
                                
                                Rectangle {
                                    width: 100
                                    height: 100
                                    radius: 20
                                    color: primaryColor
                                    
                                    Text {
                                        text: "⚡"
                                        font.pixelSize: 40
                                        anchors.centerIn: parent
                                    }
                                }
                                
                                Column {
                                    spacing: 10
                                    anchors.verticalCenter: parent.verticalCenter
                                    
                                    Text {
                                        text: qsTr("快速传输")
                                        font {
                                            pixelSize: 28
                                            weight: Font.Bold
                                        }
                                        color: textPrimary
                                    }
                                    
                                    Text {
                                        text: "Fast Transfer Pro"
                                        font.pixelSize: 16
                                        color: textSecondary
                                    }
                                    
                                    Text {
                                        text: qsTr("版本号: ") + settings_model.appVersion
                                        font.pixelSize: 14
                                        color: textSecondary
                                    }
                                    
                                    Text {
                                        text: qsTr("© 2024 快速传输团队 版权所有")
                                        font.pixelSize: 12
                                        color: textSecondary
                                    }
                                }
                            }
                        }
                        
                        Rectangle {
                            width: parent.width
                            height: 260
                            radius: 16
                            color: cardColor
                            border.color: borderColor
                            border.width: 2
                            
                            Column {
                                anchors.fill: parent
                                anchors.margins: 20
                                spacing: 15
                                
                                Row {
                                    width: parent.width
                                    spacing: 12
                                    
                                    
                                    Column {
                                        spacing: 2
                                        anchors.verticalCenter: parent.verticalCenter
                                        
                                        Text {
                                            text: qsTr("检查更新")
                                            font {
                                                pixelSize: 18
                                                weight: Font.Bold
                                            }
                                            color: textPrimary
                                        }
                                        
                                        Text {
                                            text: qsTr("检查软件最新版本")
                                            font.pixelSize: 13
                                            color: textSecondary
                                        }
                                    }
                                }
                                
                                Row {
                                    spacing: 20
                                    
                                    Rectangle {
                                        width: 150
                                        height: 45
                                        radius: 10
                                        color: primaryColor
                                        
                                        Row {
                                            spacing: 8
                                            anchors.centerIn: parent
                                            
                                            
                                            Text {
                                                text: qsTr("检查更新")
                                                font.pixelSize: 16
                                                color: whiteColor
                                                anchors.verticalCenter: parent.verticalCenter
                                            }
                                        }
                                        
                                        MouseArea {
                                            anchors.fill: parent
                                            cursorShape: Qt.PointingHandCursor
                                            hoverEnabled: true
                                            onClicked: checkForUpdates()
                                            
                                            onEntered: parent.opacity = 0.9
                                            onExited: parent.opacity = 1
                                        }
                                    }
                                    
                                    Text {
                                        text: settings_model.isUpdateAvailable ? qsTr("发现新版本") : qsTr("当前已是最新版本")
                                        font.pixelSize: 14
                                        color: settings_model.isUpdateAvailable ? accentOrange : accentGreen
                                        anchors.verticalCenter: parent.verticalCenter
                                    }
                                }
                                
                                Rectangle {
                                    width: parent.width
                                    height: 100
                                    radius: 10
                                    color: updateInfoBg
                                    border.color: borderColor
                                    border.width: 1
                                    
                                    Column {
                                        anchors.centerIn: parent
                                        spacing: 8
                                        
                                        Text {
                                            text: settings_model.appVersion + qsTr(" 更新内容")
                                            font.pixelSize: 14
                                            font.weight: Font.Bold
                                            color: textPrimary
                                        }
                                        
                                        Text {
                                            text: qsTr("• 新增主题切换功能\n• 优化传输性能\n• 修复已知问题")
                                            font.pixelSize: 12
                                            color: textSecondary
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}