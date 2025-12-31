#include <QtWidgets/QApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include <QtQuickControls2/QQuickStyle>
#include <QtNetwork/QLocalSocket>
#include <QtNetwork/QLocalServer>
#include <QtWidgets/QMessageBox>
#include <QtGui/QWindow>
// model
#include "model/ModelManager.h"
// control
#include "control/EventBusManager.h"
#include "control/NetworkController.h"
#include "control/FileSyncEngine/FileSyncEngine.h"
#include "control/SettingsController.h"
// common
#include "common/DebugOutputer.h"

#include "thirdparty/nlohmann/json.hpp"
#include <QtGui/QFont>

static const QString settingsPath = "settings.json";

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
    // 发送文件传输引擎监听完成
    EventBusManager::instance().registerEvent("/file/send_init_file_receiver_done");
    // 收到对方文件传输监听完成
    EventBusManager::instance().registerEvent("/file/have_init_file_receiver_done");
    // 关闭文件传输引擎
    EventBusManager::instance().registerEvent("/file/close_FileSyncCore");
    // 发送获取文件请求
    EventBusManager::instance().registerEvent("/file/send_get_file");
    // 收到下载请求
    EventBusManager::instance().registerEvent("/file/have_download_request");
    // 向发送队列添加任务
    EventBusManager::instance().registerEvent("/file/have_file_to_send");
    // 取消文件发送，交给文件引擎，移除等待任务
    EventBusManager::instance().registerEvent("/file/cancel_file_send");
    // 发起取消文件传输(发送方)，交给文件引擎，中断传输
    EventBusManager::instance().registerEvent("/file/cancel_transit_in_sender");
    // 收到取消文件传输(接收方)，发送方已取消发送，返回来更新状态
    EventBusManager::instance().registerEvent("/file/have_cancel_transit");
    // 发送文件取消的同步消息
    EventBusManager::instance().registerEvent("/file/send_cancel_file_send");
    // 上传进度更新
    EventBusManager::instance().registerEvent("/file/upload_progress");
    // 下载进度更新
    EventBusManager::instance().registerEvent("/file/download_progress");
    // 发送任务数修改
    EventBusManager::instance().registerEvent("/settings/send_concurrent_changed");
    // 收到任务数修改消息
    EventBusManager::instance().registerEvent("/settings/have_concurrent_changed");
    // 获取设置组配置
    EventBusManager::instance().registerEvent("/settings/get_item_config");
    // 收到设置组配置
    EventBusManager::instance().registerEvent("/settings/item_config_reslut");
    // 更新配置中键值
    EventBusManager::instance().registerEvent("/settings/update_settings_value");
    // 立即写入到文件
    EventBusManager::instance().registerEvent("/settings/write_into_file");
}

void repairSettingsFile()
{
    QFile res_file(":/settings/settings.json");
    if (res_file.exists())
    {
        QFile::remove(settingsPath);
    }
    if (res_file.copy(settingsPath))
    {
        QFile::setPermissions(settingsPath,
                              QFile::ReadOwner | QFile::WriteOwner |
                                  QFile::ReadGroup | QFile::ReadOther);
    }
    else
    {
        LOG_ERROR("Failed to copy file:" << res_file.errorString().toStdString());
    }
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    LOG_INFO("tmp dir " << GlobalStatusManager::absolute_tmp_dir);

    QApplication app(argc, argv);
    app.setApplicationName("XFileTransit");
    app.setOrganizationName("Xqqyt");
    app.setWindowIcon(QIcon(":/logo/logo_small.ico"));

    QQuickStyle::setStyle("Basic");

    QFont defaultFont("Segoe UI");
    app.setFont(defaultFont);

    // 检测是否已有一个实例在运行
    const QString appId = "XFileTransit_unique_app_id_" + app.applicationName();

    QLocalSocket socket;
    socket.connectToServer(appId);

    if (socket.waitForConnected(500))
    {
        // 已有一个实例在运行
        socket.close();
        QMessageBox::information(nullptr, "程序已在运行",
                                 "XFileTransit 已在运行中。\n请检查系统托盘或任务栏。");
        return 0;
    }

    QLocalServer::removeServer(appId);

    QLocalServer server;
    if (!server.listen(appId))
    {
        QMessageBox::critical(nullptr, "错误",
                              "无法创建应用程序实例，请检查系统权限。");
        return -1;
    }

    QDir::setCurrent(QCoreApplication::applicationDirPath());

    QQmlApplicationEngine engine;
    // 初始化组件
    EventBusManager::instance().startEventBus();
    initRegisterEvents();
    if (!QFile::exists(settingsPath))
    {
        repairSettingsFile();
    }

    NetworkController network_controller;
    FileSyncEngine file_sync_engine;
    SettingsController settings_controller;
    // 修复配置文件
    try
    {
        settings_controller.loadSettingsFromFile();
    }
    catch (nlohmann::json_abi_v3_12_0::detail::parse_error)
    {
        repairSettingsFile();
        settings_controller.loadSettingsFromFile();
        QMessageBox::information(nullptr, "提示",
                                 "配置文件损坏，已恢复默认设置");
    }

    // 创建模型实例
    auto file_list_model = ModelManager::getInstance().getFileListModel();
    auto net_list_model = ModelManager::getInstance().getNetworkInfoModel();
    auto device_list_model = ModelManager::getInstance().getDeviceModel();
    auto connection_manager = ModelManager::getInstance().getConnectionManager();
    auto settings_model = ModelManager::getInstance().getSettingsModel();
    settings_model->setQmlEngine(&engine);

    // 注册到 QML 上下文
    engine.rootContext()->setContextProperty("file_list_model", file_list_model.get());
    engine.rootContext()->setContextProperty("net_info_list_model", net_list_model.get());
    engine.rootContext()->setContextProperty("device_list_model", device_list_model.get());
    engine.rootContext()->setContextProperty("connection_manager", connection_manager.get());
    engine.rootContext()->setContextProperty("settings_model", settings_model.get());

    // 加载 QML
    engine.load(QUrl("qrc:/ui/MainWindow.qml"));

    if (engine.rootObjects().isEmpty())
    {
        server.close();
        QLocalServer::removeServer(appId);
        return -1;
    }

    QObject::connect(&app, &QApplication::aboutToQuit, [&server, appId]()
                     {
        server.close();
        QLocalServer::removeServer(appId); });

    return app.exec();
}