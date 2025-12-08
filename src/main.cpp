#include <QtWidgets/QApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include <QtQuickControls2/QQuickStyle>
// model
#include "model/ModelManager.h"
// control
#include "control/EventBusManager.h"
#include "control/NetworkController.h"
#include "control/FileSyncEngine/FileSyncEngine.h"
// common
#include "common/DebugOutputer.h"

#include <QtGui/QFont>

void initRegisterEvents()
{
    // 发送连接请求
    EventBusManager::instance().registerEvent("/network/send_connect_request");
    // 收到连接请求
    EventBusManager::instance().registerEvent("/network/have_connect_request");
    // 发送请求结果，接受/拒绝
    EventBusManager::instance().registerEvent("/network/send_connect_request_result");
    // 重置底层连接
    EventBusManager::instance().registerEvent("/network/reset_connection");
    // 收到取消连接
    EventBusManager::instance().registerEvent("/network/cancel_conn_request");
    // 收到请求结果，接受/拒绝
    EventBusManager::instance().registerEvent("/network/have_connect_request_result");
    // 断开所有连接
    EventBusManager::instance().registerEvent("/network/disconnect");
    // 收到驱动connect的错误
    EventBusManager::instance().registerEvent("/network/have_connect_error");
    // 收到驱动recv的错误
    EventBusManager::instance().registerEvent("/network/have_recv_error");
    // 收到对端关闭连接
    EventBusManager::instance().registerEvent("/network/connection_closed");
    // 发送添加文件消息
    EventBusManager::instance().registerEvent("/sync/send_addfiles");
    // 收到添加文件消息
    EventBusManager::instance().registerEvent("/sync/have_addfiles");
    // 发送删除文件消息
    EventBusManager::instance().registerEvent("/sync/send_deletefiles");
    // 收到删除文件消息
    EventBusManager::instance().registerEvent("/sync/have_deletefiles");
    // 发送失效文件
    EventBusManager::instance().registerEvent("/sync/send_expired_file");
    // 受到失效文件
    EventBusManager::instance().registerEvent("/sync/have_expired_file");
    // 初始化文件传输引擎
    EventBusManager::instance().registerEvent("/file/initialize_FileSyncCore");
    // 关闭文件传输引擎
    EventBusManager::instance().registerEvent("/file/close_FileSyncCore");
    // 发送获取文件请求
    EventBusManager::instance().registerEvent("/file/send_get_file");
    // 收到下载请求
    EventBusManager::instance().registerEvent("/file/have_download_request");
    // 向发送队列添加任务
    EventBusManager::instance().registerEvent("/file/have_file_to_send");
    // 上传进度更新
    EventBusManager::instance().registerEvent("/file/upload_progress");
    // 下载进度更新
    EventBusManager::instance().registerEvent("/file/download_progress");
}
int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    LOG_INFO("tmp dir " << GlobalStatusManager::absolute_tmp_dir);

    QApplication app(argc, argv);
    app.setApplicationName("Xqqyt");
    app.setOrganizationName("XQQYT");
    app.setWindowIcon(QIcon(":/logo/logo/logo_small.ico"));

    QQuickStyle::setStyle("Basic");

    QFont defaultFont("Segoe UI");
    app.setFont(defaultFont);

    QQmlApplicationEngine engine;
    // 初始化组件
    EventBusManager::instance().startEventBus();
    initRegisterEvents();

    NetworkController network_controller;
    FileSyncEngine file_sync_engine;

    // 创建模型实例
    auto file_list_model = ModelManager::getInstance().getFileListModel();
    auto net_list_model = ModelManager::getInstance().getNetworkInfoModel();
    auto device_list_model = ModelManager::getInstance().getDeviceModel();
    auto connection_manager = ModelManager::getInstance().getConnectionManager();

    // 注册到 QML 上下文
    engine.rootContext()->setContextProperty("file_list_model", file_list_model.get());
    engine.rootContext()->setContextProperty("net_info_list_model", net_list_model.get());
    engine.rootContext()->setContextProperty("device_list_model", device_list_model.get());
    engine.rootContext()->setContextProperty("connection_manager", connection_manager.get());

    // 加载 QML
    engine.load(QUrl("qrc:/qml/ui/MainWindow.qml"));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}