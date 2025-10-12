#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include "model/FileListModel.h"

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);


    QQmlApplicationEngine engine;
    FileListModel file_model;
    engine.rootContext()->setContextProperty("file_list_model", &file_model);
    engine.load(QUrl("qrc:/ui/MainWindow/MainWindow.qml"));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
