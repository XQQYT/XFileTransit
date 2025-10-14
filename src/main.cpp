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