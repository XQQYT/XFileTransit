#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include "model/FileListModel.h"
#include "model/DeviceListModel.h"

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    // 创建模型实例
    FileListModel* file_list_model = new FileListModel(&engine);
    DeviceListModel* device_list_model = new DeviceListModel(&engine);

    // 注册到 QML 上下文
    engine.rootContext()->setContextProperty("file_list_model", file_list_model);
    engine.rootContext()->setContextProperty("device_list_model", device_list_model);

    // 加载 QML
    engine.load(QUrl("qrc:/qml/ui/MainWindow.qml"));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}

// #include <QtCore/QCoreApplication>
// #include <QtCore/QDebug>
// #include "model/ICMPScanner.h"
// #include "model/DeviceListModel.h"

// int main(int argc, char* argv[])
// {
//     QCoreApplication app(argc, argv);

//     // 创建扫描器实例
//     ICMPScanner scanner;

//     scanner.setNetworkRange("192.168.16.1/24");

//     QObject::connect(&scanner, &ICMPScanner::scanProgress, [](int percent) {
//         qDebug() << "扫描进度:" << percent << "%";
//         });

//     QObject::connect(&scanner, &ICMPScanner::scanFinished, [](const QVector<DeviceInfo>& devices_result) {
//         for (auto& i : devices_result) {
//             qDebug() << i.device_ip << i.device_name << i.device_type;
//         }
//         QCoreApplication::quit();
//         });

//     QObject::connect(&scanner, &ICMPScanner::scanError, [](const QString& error) {
//         qDebug() << "扫描错误:" << error;
//         QCoreApplication::quit();
//         });

//     // 设置扫描参数
//     // scanner.setNetworkRange("192.168.1.0/24"); // 扫描整个C类网络
//     scanner.setTimeout(1000); // 1秒超时
//     scanner.setThreadCount(10); // 10个线程并行扫描

//     // 开始扫描
//     qDebug() << "开始局域网扫描...";
//     scanner.startScan();

//     return app.exec();
// }