#ifndef _WEBSOCKET_H
#define _WEBSOCKET_H

#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <stdexcept>
#include <memory>
#include "driver/interface/NetworkInterface.h"

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
using tcp = asio::ip::tcp;

class WebSocket : public NetworkInterface, public std::enable_shared_from_this<WebSocket>
{
public:
    WebSocket();
    WebSocket(const WebSocket &obj) = delete;
    WebSocket &operator=(WebSocket &obj) = delete;
    void setNetworkInfo(const std::string &address, const std::string &port) override;
    void connectTo(std::function<void(bool)> callback = nullptr) override;
    void sendMsg(const std::string &msg) override;
    void recvMsg(std::function<void(std::unique_ptr<UserMsg>)> callback) override;
    void closeSocket() override;
    void resetConnection() override;
    ~WebSocket();

private:
    std::optional<asio::executor_work_guard<asio::io_context::executor_type>> work_guard;
    std::thread io_thread;
    std::unique_ptr<asio::io_context> ioc;
    std::unique_ptr<websocket::stream<tcp::socket>> ws_socket;
    std::unique_ptr<tcp::resolver> resolver;

    std::string address;
    std::string port;
};

#endif