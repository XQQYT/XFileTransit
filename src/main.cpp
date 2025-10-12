#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include "model/FileListModel.h"

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    // 创建模型实例
    FileListModel* fileModel = new FileListModel(&engine);

    // 注册到 QML 上下文
    engine.rootContext()->setContextProperty("file_list_model", fileModel);

    // 加载 QML
    engine.load(QUrl("qrc:/qml/ui/MainWindow.qml"));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}