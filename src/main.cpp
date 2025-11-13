#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
// model
#include "model/ModelManager.h"
#include "model/FileListModel.h"
#include "model/DeviceListModel.h"
// control
#include "control/EventBusManager.h"
#include "control/NetworkController.h"

#include <QtGui/QFont>

void initRegisterEvents()
{
    EventBusManager::instance().registerEvent("/network/send_connect_request");
    EventBusManager::instance().registerEvent("/network/have_connect_request");

}
int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    QFont defaultFont("Segoe UI");
    app.setFont(defaultFont);

    QQmlApplicationEngine engine;

    // 初始化组件
    EventBusManager::instance().startEventBus();
    initRegisterEvents();

    NetworkController network_controller;

    // 创建模型实例
    auto file_list_model = ModelManager::getInstance().getFileListModel();
    auto device_list_model = ModelManager::getInstance().getDeviceModel();
    auto connection_manager = ModelManager::getInstance().getConnectionManager();
    file_list_model->setParent(&engine);
    device_list_model->setParent(&engine);

    // 注册到 QML 上下文
    engine.rootContext()->setContextProperty("file_list_model", file_list_model.get());
    engine.rootContext()->setContextProperty("device_list_model", device_list_model.get());
    engine.rootContext()->setContextProperty("connection_manager", connection_manager.get());

    // 加载 QML
    engine.load(QUrl("qrc:/qml/ui/MainWindow.qml"));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}