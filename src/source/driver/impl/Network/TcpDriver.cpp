#include "driver/impl/Network/TcpDriver.h"
#include "driver/impl/OuterMsgBuilder.h"
#include "driver/impl/OuterMsgParser.h"
#include "driver/interface/PlatformSocket.h"
#include "common/DebugOutputer.h"
#include <iostream>
#include <memory>
#include <iomanip>
#include <cstring>
#include <cerrno>
#include <chrono>

#ifdef _WIN32
#else
#include <unistd.h>
#include <sys/select.h>
#include <poll.h>
#endif

TcpDriver::TcpDriver()
    : msg_builder(std::make_unique<OuterMsgBuilder>(security_instance)),
      msg_parser(std::make_unique<OuterMsgParser>()),
      connection_status(ConnectionStatus::WAITING_TLS),
      client_tls_addr({}),
      client_tcp_addr({}),
      receive_thread(nullptr),
      tls_listen_thread(nullptr),
      tcp_listen_thread(nullptr)
{
#ifdef _WIN32
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
    {
        LOG_ERROR("WSAStartup failed\n");
    }
#endif
}

TcpDriver::~TcpDriver()
{
    closeSocket();
}

void TcpDriver::setTlsNetworkInfo(const std::string &address, const std::string &tls_port)
{
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET_VAL)
    {
        LOG_ERROR("Failed to create TLS socket: " << GET_SOCKET_ERROR);
        return;
    }

    client_tls_addr.sin_family = AF_INET;
    client_tls_addr.sin_port = htons(static_cast<uint16_t>(std::stoi(tls_port)));

    if (inet_pton(AF_INET, address.c_str(), &client_tls_addr.sin_addr) <= 0)
    {
        LOG_ERROR("Invalid TLS address: " << address);
        CLOSE_SOCKET(client_socket);
        client_socket = INVALID_SOCKET_VAL;
    }
}

void TcpDriver::dealConnectError()
{
    if (ignore_one_error)
    {
        ignore_one_error = false;
        return;
    }

    if (this->dce_cb)
    {
        int error_code = GET_SOCKET_ERROR;
        switch (error_code)
        {
        case SOCKET_ECONNREFUSED: // Windows: WSAECONNREFUSED, Linux: ECONNREFUSED
            this->dce_cb(ConnectError::CONNECT_REFUSED);
            break;
        case SOCKET_ETIMEDOUT:
            this->dce_cb(ConnectError::CONNECT_TIMEOUT);
            break;
        case SOCKET_EHOSTUNREACH:
            this->dce_cb(ConnectError::CONNECT_HOST_UNREACHABLE);
            break;
        case SOCKET_ENETUNREACH:
            this->dce_cb(ConnectError::CONNECT_NETWORK_UNREACHABLE);
            break;
        case SOCKET_EADDRINUSE:
            this->dce_cb(ConnectError::CONNECT_ADDR_IN_USE);
            break;
        case SOCKET_EINPROGRESS:
            this->dce_cb(ConnectError::CONNECT_IN_PROGRESS);
            break;
        case SOCKET_EACCES:
            this->dce_cb(ConnectError::CONNECT_ACCESS_DENIED);
            break;
        case SOCKET_EISCONN:
            this->dce_cb(ConnectError::CONNECT_ALREADY_CONNECTED);
            break;
        case SOCKET_EFAULT:
            this->dce_cb(ConnectError::CONNECT_BAD_ADDRESS);
            break;
        case SOCKET_EINTR:
            this->dce_cb(ConnectError::CONNECT_INTERRUPTED);
            break;
        default:
            LOG_ERROR("Connect unknown error: " << error_code);
            break;
        }
    }
}

void TcpDriver::connect(const std::string &address, const std::string &port, std::function<void(bool)> callback)
{
    LOG_DEBUG("tcp connect!!!");
    client_tcp_addr.sin_family = AF_INET;
    client_tcp_addr.sin_port = htons(static_cast<uint16_t>(std::stoi(port)));

    if (inet_pton(AF_INET, address.c_str(), &client_tcp_addr.sin_addr) <= 0)
    {
        LOG_ERROR("Invalid TCP address: " << address);
    }

    ignore_one_error = false;

    if (client_socket == INVALID_SOCKET_VAL)
    {
        LOG_ERROR("Socket not initialized");
        if (callback)
            callback(false);
        return;
    }

    int ret = ::connect(client_socket, (sockaddr *)&client_tls_addr, sizeof(client_tls_addr));
    if (ret == SOCKET_ERROR_VAL)
    {
        dealConnectError();
    }
    else
    {
        LOG_INFO("Connect successful");
    }

    if (security_instance && ret == 0)
    {
        // 发送建立TLS请求
        security_instance->setTlsInfo(security_instance->getAesKey(client_socket));

        // TLS连接完成，发起普通tcp连接
        CLOSE_SOCKET(client_socket);
        client_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (client_socket == INVALID_SOCKET_VAL)
        {
            LOG_ERROR("Failed to create new socket for TCP connection");
            if (callback)
                callback(false);
            return;
        }

        ret = ::connect(client_socket, (sockaddr *)&client_tcp_addr, sizeof(client_tcp_addr));
    }

    connect_status = (ret == 0);

    if (callback)
    {
        callback(ret == 0);
    }
}

void printHex(const std::vector<uint8_t> &data)
{
    for (size_t i = 0; i < data.size(); ++i)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<int>(data[i]) << " ";
    }
    std::cout << std::dec << std::endl;
}

void TcpDriver::sendMsg(const std::string &msg, std::string)
{
    if (!connect_status)
    {
        LOG_ERROR("Not connected, cannot send message");
        return;
    }

    NetworkInterface::Flag flag = static_cast<NetworkInterface::Flag>(0);
    std::unique_ptr<NetworkInterface::UserMsg> ready_to_send_msg = msg_builder->buildMsg(msg, flag);

    if (!ready_to_send_msg)
    {
        LOG_ERROR("Failed to build message");
        return;
    }

    size_t final_msg_length = ready_to_send_msg->data.size();
    size_t sended_length = 0;

    while (sended_length < final_msg_length)
    {
        int ret = send(client_socket,
                       reinterpret_cast<const char *>(ready_to_send_msg->data.data() + sended_length),
                       static_cast<int>(final_msg_length - sended_length), 0);
        if (ret <= 0)
        {
            int err = GET_SOCKET_ERROR;
            if (err == SOCKET_EINTR)
            {
                continue; // 被信号中断，重试
            }
            LOG_ERROR("Send failed, error: " << err);
            break;
        }
        sended_length += ret;
        LOG_INFO(sended_length << " / " << final_msg_length);
    }
}

UnifiedSocket TcpDriver::createListenSocket(const std::string &address, const std::string &port)
{
    UnifiedSocket sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET_VAL)
    {
        LOG_ERROR("Failed to create listen socket: " << GET_SOCKET_ERROR);
        return INVALID_SOCKET_VAL;
    }

    // 设置SO_REUSEADDR
    int opt = 1;
#ifdef _WIN32
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));
#else
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(std::stoi(port)));

    if (address.empty() || address == "0.0.0.0")
    {
        addr.sin_addr.s_addr = INADDR_ANY;
    }
    else
    {
        if (inet_pton(AF_INET, address.c_str(), &addr.sin_addr) <= 0)
        {
            LOG_ERROR("Invalid listen address: " << address);
            CLOSE_SOCKET(sock);
            return INVALID_SOCKET_VAL;
        }
    }

    if (bind(sock, (sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR_VAL)
    {
        LOG_ERROR("Bind failed: " << GET_SOCKET_ERROR);
        CLOSE_SOCKET(sock);
        return INVALID_SOCKET_VAL;
    }

    if (listen(sock, 5) == SOCKET_ERROR_VAL)
    {
        LOG_ERROR("Listen failed: " << GET_SOCKET_ERROR);
        CLOSE_SOCKET(sock);
        return INVALID_SOCKET_VAL;
    }

    LOG_INFO("Listen socket created on " << address << ":" << port);
    return sock;
}

#ifdef _WIN32
static bool checkWindowsEvent(HANDLE event, DWORD timeout = 0)
{
    return WaitForSingleObject(event, timeout) == WAIT_OBJECT_0;
}
#else
static bool checkUnixEvent(int fd, int timeout_ms = 0)
{
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;
    return poll(&pfd, 1, timeout_ms) > 0 && (pfd.revents & POLLIN);
}
#endif

void TcpDriver::startTlsListen(const std::string &address, const std::string &tls_port,
                               std::function<bool(bool)> tls_callback)
{
    tls_listen_socket = createListenSocket(address, tls_port);
    if (tls_listen_socket == INVALID_SOCKET_VAL)
    {
        if (tls_callback)
            tls_callback(false);
        return;
    }

    tls_listen_thread = new std::thread([this, cb = std::move(tls_callback)]()
                                        {
#ifdef _WIN32
        WSAPOLLFD fds[1];
        fds[0].fd = tls_listen_socket;
        fds[0].events = POLLRDNORM;
#else
        pollfd fds[1];
        fds[0].fd = tls_listen_socket;
        fds[0].events = POLLIN;
#endif

        while (this->listen_running.load())
        {
            // 只有在等待TLS请求时才接收
            if (this->connection_status == ConnectionStatus::WAITING_TLS)
            {
#ifdef _WIN32
                int result = WSAPoll(fds, 1, 50);
#else
                int result = poll(fds, 1, 50);
#endif

                if (result > 0) {
#ifdef _WIN32
                    bool ready = (fds[0].revents & POLLRDNORM) != 0;
#else
                    bool ready = (fds[0].revents & POLLIN) != 0;
#endif
                    if (ready)
                    {
                        sockaddr_in client_addr;
                        socklen_t client_addr_len = sizeof(client_addr);
                        UnifiedSocket accepted_socket = accept(tls_listen_socket, 
                                                             (sockaddr*)&client_addr, 
                                                             &client_addr_len);
                        if (accepted_socket != INVALID_SOCKET_VAL)
                        {
                            if (security_instance)
                            {
                                char client_ip[INET_ADDRSTRLEN];
                                inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
                                LOG_INFO("TLS connection from: " << client_ip );

                                security_instance->dealTlsRequest(accepted_socket, 
                                    [this, accepted_socket, &cb, &client_addr](bool ret, SecurityInterface::TlsInfo info) {
                                        if (ret)
                                        {
                                            candidate_ip = inet_ntoa(client_addr.sin_addr);
                                            security_instance->setTlsInfo(info);
                                            connection_status = ConnectionStatus::TLS_CONNECTED;
                                        }
                                        CLOSE_SOCKET(accepted_socket);
                                        if (cb) cb(ret);
                                    });
                            } else {
                                CLOSE_SOCKET(accepted_socket);
                            }
                        } else {
                            LOG_ERROR("Failed to accept TLS connection: " << GET_SOCKET_ERROR);
                        }
                    }
                } else if (result < 0) {
                    LOG_ERROR("Error in poll for TLS: " << GET_SOCKET_ERROR);
                }
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }

        CLOSE_SOCKET(tls_listen_socket);
        LOG_INFO("TLS listen thread exited" ); });
}

void TcpDriver::startTcpListen(const std::string &address, const std::string &tcp_port,
                               std::function<bool(bool)> tcp_callback)
{
    tcp_listen_socket = createListenSocket(address, tcp_port);
    if (tcp_listen_socket == INVALID_SOCKET_VAL)
    {
        if (tcp_callback)
            tcp_callback(false);
        return;
    }

    tcp_listen_thread = new std::thread([this, cb = std::move(tcp_callback)]()
                                        {
#ifdef _WIN32
        WSAPOLLFD fds[1];
        fds[0].fd = tcp_listen_socket;
        fds[0].events = POLLRDNORM;
#else
        pollfd fds[1];
        fds[0].fd = tcp_listen_socket;
        fds[0].events = POLLIN;
#endif

        while (this->listen_running.load())
        {
            // 只有在TLS建立成功时才监听
            if (this->connection_status == ConnectionStatus::TLS_CONNECTED)
            {
#ifdef _WIN32
                int result = WSAPoll(fds, 1, 100);
#else
                int result = poll(fds, 1, 100);
#endif

                if (result > 0) {
#ifdef _WIN32
                    bool ready = (fds[0].revents & POLLRDNORM) != 0;
#else
                    bool ready = (fds[0].revents & POLLIN) != 0;
#endif
                    if (ready)
                    {
                        sockaddr_in client_addr;
                        socklen_t client_addr_len = sizeof(client_addr);
                        UnifiedSocket accepted_socket = accept(tcp_listen_socket, 
                                                             (sockaddr*)&client_addr, 
                                                             &client_addr_len);
                        if (accepted_socket != INVALID_SOCKET_VAL)
                        {
                            char client_ip[INET_ADDRSTRLEN];
                            inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
                            uint16_t client_port = ntohs(client_addr.sin_port);

                            LOG_INFO("TCP connection from: " << client_ip << ":" << client_port);

                            if (candidate_ip == client_ip)
                            {
                                client_socket = accepted_socket;
                                connect_status = true;
                                if (cb) cb(true);
                                this->connection_status = ConnectionStatus::TCP_ESTABLISHED;
                                LOG_INFO("TCP connection established with: " << client_ip );
                            } else {
                                LOG_INFO("TCP connection from different IP, expected: " 
                                          << candidate_ip << ", got: " << client_ip );
                                CLOSE_SOCKET(accepted_socket);
                            }
                        } else {
                            LOG_ERROR("Failed to accept TCP connection: " << GET_SOCKET_ERROR);
                        }
                    }
                } else if (result < 0) {
                    LOG_ERROR("Error in poll for TCP: " << GET_SOCKET_ERROR);
                }
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }

        CLOSE_SOCKET(tcp_listen_socket);
        LOG_INFO("TCP listen thread exited"); });
}

void TcpDriver::startListen(const std::string &address, const std::string &tls_port,
                            const std::string &tcp_port,
                            std::function<bool(bool)> tls_callback,
                            std::function<bool(bool)> tcp_callback)
{
    listen_running = true;
    if (security_instance)
    {
        startTlsListen(address, tls_port, tls_callback);
    }
    startTcpListen(address, tcp_port, tcp_callback);
}

void TcpDriver::recvMsg(std::function<void(std::unique_ptr<UserMsg> parsed_msg)> callback)
{
    if (!connect_status.load())
    {
        throw std::runtime_error("Not connected");
    }

    recv_running = true;
    receive_thread = new std::thread([this, callback = std::move(callback)]()
                                     {
        while (this->recv_running)
        {
            msg_parser->delegateRecv(client_socket, 
                callback, 
                this->dcc_cb, 
                this->dre_cb, 
                security_instance, 
                recv_running);
        }
        LOG_INFO("Receive thread exited"); });
}

void TcpDriver::closeSocket()
{
    listen_running = false;
    recv_running = false;

    // 等待线程结束
    if (tls_listen_thread)
    {
        if (tls_listen_thread->joinable())
            tls_listen_thread->join();
        delete tls_listen_thread;
        tls_listen_thread = nullptr;
    }
    if (tcp_listen_thread)
    {
        if (tcp_listen_thread->joinable())
            tcp_listen_thread->join();
        delete tcp_listen_thread;
        tcp_listen_thread = nullptr;
    }
    if (receive_thread)
    {
        if (receive_thread->joinable())
            receive_thread->join();
        delete receive_thread;
        receive_thread = nullptr;
    }

    // 关闭socket
    if (client_socket != INVALID_SOCKET_VAL)
    {
        CLOSE_SOCKET(client_socket);
        client_socket = INVALID_SOCKET_VAL;
    }
    if (tls_listen_socket != INVALID_SOCKET_VAL)
    {
        CLOSE_SOCKET(tls_listen_socket);
        tls_listen_socket = INVALID_SOCKET_VAL;
    }
    if (tcp_listen_socket != INVALID_SOCKET_VAL)
    {
        CLOSE_SOCKET(tcp_listen_socket);
        tcp_listen_socket = INVALID_SOCKET_VAL;
    }

    connect_status = false;

#ifdef _WIN32
    WSACleanup();
#endif

    LOG_INFO("Socket closed");
}

void TcpDriver::setSecurityInstance(std::shared_ptr<SecurityInterface> instance)
{
    security_instance = instance;
    if (msg_builder)
    {
        msg_builder->setSecurityInstance(instance);
    }
}

void TcpDriver::resetConnection()
{
    if (connect_status)
    {
        closesocket(client_socket);
    }
    recv_running = false;
    client_socket = INVALID_SOCKET_VAL;
    candidate_ip.clear();
    client_tls_addr = {};
    client_tcp_addr = {};
    accept_addr = {};
    ignore_one_error = true;
    connection_status = ConnectionStatus::WAITING_TLS;

    LOG_INFO("Connection reset");
}