#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
// model
#include "model/ModelManager.h"
// control
#include "control/EventBusManager.h"
#include "control/NetworkController.h"

#include <QtGui/QFont>

void initRegisterEvents()
{
    //发送连接请求
    EventBusManager::instance().registerEvent("/network/send_connect_request");
    //收到连接请求
    EventBusManager::instance().registerEvent("/network/have_connect_request");
    //发送请求结果，接受/拒绝
    EventBusManager::instance().registerEvent("/network/send_connect_request_result");
    //收到请求结果，接受/拒绝
    EventBusManager::instance().registerEvent("/network/have_connect_request_result");
    //断开所有连接
    EventBusManager::instance().registerEvent("/network/disconnect");
    //收到驱动connect的错误
    EventBusManager::instance().registerEvent("/network/have_connect_error");
    //收到驱动recv的错误
    EventBusManager::instance().registerEvent("/network/have_recv_error");
    //收到对端关闭连接
    EventBusManager::instance().registerEvent("/network/connection_closed");

    //发送添加文件消息
    EventBusManager::instance().registerEvent("/sync/send_addfiles");
    //收到添加文件消息
    EventBusManager::instance().registerEvent("/sync/have_addfiles");
    //发送删除文件消息
    EventBusManager::instance().registerEvent("/sync/send_deletefiles");
    //收到删除文件消息
    EventBusManager::instance().registerEvent("/sync/have_deletefiles");
}
int main(int argc, char* argv[])
{
    QTextStream(stdout).setEncoding(QStringConverter::Encoding::Utf8);

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