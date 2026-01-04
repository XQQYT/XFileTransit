#include "driver/impl/P2PDriver.h"
#include "driver/impl/WebSocket.h"
#include "common/DebugOutputer.h"

P2PDriver::P2PDriver()
    : websocket_driver(WebSocket::create())
{
}

// 用户数据
void P2PDriver::setNetworkInfo(const std::string &address, const std::string &port)
{
    signal_address = address;
    signal_port = port;
    websocket_driver->setNetworkInfo(signal_address, signal_port);
}

void P2PDriver::connectTo(std::function<void(bool)> callback)
{
    websocket_driver->connectTo(callback);
}

void P2PDriver::sendMsg(const std::string &msg)
{
    websocket_driver->sendMsg(msg);
}

void P2PDriver::recvMsg(std::function<void(std::string)> callback)
{
    websocket_driver->recvMsg(callback);
}

void P2PDriver::closeSocket()
{
}

void P2PDriver::resetConnection()
{
}

// 文件接收接口
void P2PDriver::start(std::function<void(UnifiedSocket)> accept_cb,
                      std::function<void(UnifiedSocket socket, std::unique_ptr<NetworkInterface::UserMsg>)> msg_cb)
{
}

// 文件发送接口
void P2PDriver::start(std::function<std::optional<std::pair<uint32_t, std::string>>()> get_task_cb)
{
}