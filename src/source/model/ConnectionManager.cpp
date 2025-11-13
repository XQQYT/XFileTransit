#include "model/ConnectionManager.h"
#include "control/EventBusManager.h"

ConnectionManager::ConnectionManager()
{
    EventBusManager::instance().subscribe("/network/have_connect_request", std::bind(&ConnectionManager::onHaveConnectRequest,
        this,
        std::placeholders::_1,
        std::placeholders::_2));
}

void ConnectionManager::onHaveConnectRequest(std::string device_ip, std::string device_name)
{
    QString di = QString::fromStdString(std::move(device_ip));
    QString dn = QString::fromStdString(std::move(device_name));
    QMetaObject::invokeMethod(this, [this, di, dn]() {
        emit haveConRequest(di, dn);
        }, Qt::QueuedConnection);
}