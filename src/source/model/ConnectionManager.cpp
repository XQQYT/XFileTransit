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
    current_target_ip = QString::fromStdString(std::move(device_ip));
    current_target_name = QString::fromStdString(std::move(device_name));
    QMetaObject::invokeMethod(this, [this]() {
        emit haveConRequest(this->current_target_ip, this->current_target_name);
    }, Qt::QueuedConnection);
}

void ConnectionManager::accepted(const QString device_ip, const QString device_name)
{
    EventBusManager::instance().publish("/network/send_connect_request_result", true);
    current_conn_status = true;
}

void ConnectionManager::rejected(const QString device_ip, const QString device_name)
{
    EventBusManager::instance().publish("/network/send_connect_request_result", false);
    current_conn_status = false;
}