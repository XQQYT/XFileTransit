#include "model/ConnectionManager.h"
#include "control/EventBusManager.h"
#include <iostream>

ConnectionManager::ConnectionManager()
{
    EventBusManager::instance().subscribe("/network/have_connect_request", std::bind(&ConnectionManager::onHaveConnectRequest,
        this,
        std::placeholders::_1,
        std::placeholders::_2));
    EventBusManager::instance().subscribe("/network/have_connect_error", std::bind(&ConnectionManager::onHaveConnectError,
        this,
        std::placeholders::_1));
    EventBusManager::instance().subscribe("/network/have_recv_error", std::bind(&ConnectionManager::onHaveRecvError,
        this,
        std::placeholders::_1));
    EventBusManager::instance().subscribe("/network/connection_closed", std::bind(&ConnectionManager::onPeerClosed,
        this));
    EventBusManager::instance().subscribe("/network/cancel_conn_request", std::bind(ConnectionManager::onCancelConnRequest,
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

void ConnectionManager::disconnect()
{
    EventBusManager::instance().publish("/network/disconnect");
}

void ConnectionManager::onHaveConnectError(std::string message)
{
    QMetaObject::invokeMethod(this, [this, message]() {
        emit haveConnectError(QString::fromStdString(message));
        emit connectionClosed();
        }, Qt::QueuedConnection);
}

void ConnectionManager::onHaveRecvError(std::string message)
{
    QMetaObject::invokeMethod(this, [this, message]() {
        emit haveRecvError(QString::fromStdString(message));
        emit connectionClosed();
        }, Qt::QueuedConnection);
}

void ConnectionManager::onPeerClosed()
{
    QMetaObject::invokeMethod(this, [this]() {
        emit peerClosed();
        emit connectionClosed();
        }, Qt::QueuedConnection);
}

void ConnectionManager::onCancelConnRequest(std::string ip, std::string name)
{
    QMetaObject::invokeMethod(this, [this, ip, name]() {
        emit conRequestCancel(QString::fromStdString(std::move(ip)), QString::fromStdString(std::move(name)));
        }, Qt::QueuedConnection);
}