#include "model/ConnectionManager.h"
#include "control/EventBusManager.h"
#include <iostream>

ConnectionManager::ConnectionManager()
{
    EventBusManager::instance().subscribe("/network/have_connect_request", std::bind(&ConnectionManager::onHaveConnectRequest,
        this,
        std::placeholders::_1,
        std::placeholders::_2));
}

void ConnectionManager::onHaveConnectRequest(std::string device_ip, std::string device_name)
{
    QMetaObject::invokeMethod(this, [this, device_ip, device_name]() {
        emit haveConRequest(QString::fromStdString(std::move(device_ip)), QString::fromStdString(std::move(device_name)));
    }, Qt::QueuedConnection);
}

void ConnectionManager::accepted(const QString device_ip, const QString device_name)
{
    EventBusManager::instance().publish("/network/send_connect_request_result", true);
}

void ConnectionManager::rejected(const QString device_ip, const QString device_name)
{
    EventBusManager::instance().publish("/network/send_connect_request_result", false);
}