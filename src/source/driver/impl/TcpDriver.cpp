#include "driver/impl/TcpDriver.h"
#include "driver/impl/OuterMsgBuilder.h"
#include "driver/impl/OuterMsgParser.h"
#include <iostream>
#include <memory.h>
#include <iomanip>

TcpDriver::TcpDriver() : msg_builder(std::make_unique<OuterMsgBuilder>(security_instance)),
msg_parser(std::make_unique<OuterMsgParser>()),
client_tls_addr({}),
client_tcp_addr({}),
receive_thread(nullptr),
tls_listen_thread(nullptr),
tcp_listen_thread(nullptr)
{
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
    {
        std::cerr << "WSAStartup failed\n";
        return;
    }
}

TcpDriver::~TcpDriver()
{
    if (client_socket)
        closeSocket();

}

void TcpDriver::initTlsSocket(const std::string& address, const std::string& tls_port)
{
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    client_tls_addr.sin_family = AF_INET;
    client_tls_addr.sin_port = htons(std::stoi(tls_port));
    inet_pton(AF_INET, address.c_str(), &client_tls_addr.sin_addr);
}

void TcpDriver::initTcpSocket(const std::string& address, const std::string& tcp_port)
{
    client_tcp_addr.sin_family = AF_INET;
    client_tcp_addr.sin_port = htons(std::stoi(tcp_port));
    inet_pton(AF_INET, address.c_str(), &client_tcp_addr.sin_addr);
}

void TcpDriver::dealConnectError()
{
    if (ignore_one_error)
    {
        ignore_one_error = !ignore_one_error;
        return;
    }
    if (this->dce_cb)
    {
        int error_code = WSAGetLastError();
        switch (error_code) {
        case WSAECONNREFUSED:
            this->dce_cb(ConnectError::CONNECT_REFUSED);
            break;
        case WSAETIMEDOUT:
            this->dce_cb(ConnectError::CONNECT_TIMEOUT);
            break;
        case WSAEHOSTUNREACH:
            this->dce_cb(ConnectError::CONNECT_HOST_UNREACHABLE);
            break;
        case WSAENETUNREACH:
            this->dce_cb(ConnectError::CONNECT_NETWORK_UNREACHABLE);
            break;
        case WSAEADDRINUSE:
            this->dce_cb(ConnectError::CONNECT_ADDR_IN_USE);
            break;
        case WSAEINPROGRESS:
            this->dce_cb(ConnectError::CONNECT_IN_PROGRESS);
            break;
        case WSAEACCES:
            this->dce_cb(ConnectError::CONNECT_ACCESS_DENIED);
            break;
        case WSAEISCONN:
            this->dce_cb(ConnectError::CONNECT_ALREADY_CONNECTED);
            break;
        case WSAEFAULT:
            this->dce_cb(ConnectError::CONNECT_BAD_ADDRESS);
            break;
        case WSAEINTR:
            this->dce_cb(ConnectError::CONNECT_INTERRUPTED);
            break;
        default:
            std::cerr << "Connect unknown error" << std::endl;
            break;
        }
    }
}


void TcpDriver::connectTo(std::function<void(bool)> callback)
{
    ignore_one_error = false;
    int ret = connect(client_socket, (sockaddr*)&client_tls_addr, sizeof(client_tls_addr));
    if (ret == SOCKET_ERROR) {
        dealConnectError();
    }
    else {
        std::cout << "Connect successful" << std::endl;
    }
    if (security_instance && !ret)
    {
        // 发送建立TLS请求
        security_instance->setTlsInfo(security_instance->getAesKey(client_socket));

        // TLS连接完成，发起普通tcp连接
        client_socket = socket(AF_INET, SOCK_STREAM, 0);

        ret = connect(client_socket, (sockaddr*)&client_tcp_addr, sizeof(client_tcp_addr));
    }

    connect_status = !ret;

    if (callback)
    {
        std::cout << ret << std::endl;
        callback(!ret);
    }
}

void printHex(const std::vector<uint8_t>& data) {
    for (size_t i = 0; i < data.size(); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(data[i]) << " ";
    }
    std::cout << std::dec << std::endl;
}

void TcpDriver::sendMsg(const std::string& msg)
{
    if (!connect_status)
        return;
    NetworkInterface::Flag flag = NetworkInterface::Flag::IS_ENCRYPT;
    std::unique_ptr<NetworkInterface::UserMsg> ready_to_send_msg = std::move(msg_builder->buildMsg(msg, flag));

    size_t final_msg_length = ready_to_send_msg->data.size();
    size_t sended_length = 0;

    while (sended_length < final_msg_length)
    {
        int ret = send(client_socket, reinterpret_cast<const char*>(ready_to_send_msg->data.data() + sended_length),
            final_msg_length - sended_length, 0);
        if (ret <= 0)
        {
            if (errno == EINTR)
                continue;
            perror("write failed");
            break;
        }
        sended_length += ret;
        std::cout << sended_length << " / " << final_msg_length << std::endl;
    }
}
SOCKET TcpDriver::createListenSocket(const std::string& address, const std::string& port) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) return INVALID_SOCKET;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(std::stoi(port));
    inet_pton(AF_INET, address.c_str(), &addr.sin_addr);

    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        closesocket(sock);
        return INVALID_SOCKET;
    }

    if (listen(sock, 5) == SOCKET_ERROR) {
        closesocket(sock);
        return INVALID_SOCKET;
    }

    return sock;
}

void TcpDriver::startTlsListen(const std::string& address, const std::string& tls_port, std::function<bool(bool)> tls_callback)
{
    //初始化tls监听
    tls_listen_socket = createListenSocket(address, tls_port);

    tls_listen_thread = new std::thread([this, cb = std::move(tls_callback)]()
        {
            WSAPOLLFD fds[1];
            fds[0].fd = tls_listen_socket;
            fds[0].events = POLLRDNORM;

            while (this->listen_running)
            {
                //只有在等待TLS请求时才接收
                if (this->connection_status == ConnectionStatus::WAITING_TLS)
                {
                    int result = WSAPoll(fds, 1, 100);

                    if (result > 0 && (fds[0].revents & POLLRDNORM))
                    {
                        int accept_addr_len = sizeof(accept_addr);
                        SOCKET accepted_socket = accept(tls_listen_socket, (sockaddr*)&accept_addr, &accept_addr_len);
                        if (accepted_socket != INVALID_SOCKET)
                        {
                            if (security_instance)
                            {
                                security_instance->dealTlsRequest(accepted_socket, [this, accepted_socket, &cb](bool ret, SecurityInterface::TlsInfo info) {
                                    if (ret)
                                    {
                                        if (accepted_socket != INVALID_SOCKET) {
                                            char client_ip[INET_ADDRSTRLEN];
                                            inet_ntop(AF_INET, &(accept_addr.sin_addr), client_ip, INET_ADDRSTRLEN);

                                            candidate_ip.assign(client_ip);
                                        }
                                        security_instance->setTlsInfo(info);
                                        connection_status = ConnectionStatus::TLS_CONNECTED;
                                    }
                                    if (cb)
                                        cb(ret);
                                    });
                            }
                        }
                        else {
                            std::cerr << "fail to accept" << WSAGetLastError() << std::endl;
                        }
                    }
                    else if (result < 0) {
                        std::cerr << "error in WSAPoll" << WSAGetLastError() << std::endl;
                    }
                }
                else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
            }

            closesocket(tls_listen_socket); });
}

void TcpDriver::startTcpListen(const std::string& address, const std::string& tcp_port, std::function<bool(bool)> tcp_callback)
{
    //初始化tcp监听
    tcp_listen_socket = createListenSocket(address, tcp_port);

    tcp_listen_thread = new std::thread([this, cb = std::move(tcp_callback)]()
        {
            WSAPOLLFD fds[1];
            fds[0].fd = tcp_listen_socket;
            fds[0].events = POLLRDNORM;

            while (this->listen_running)
            {
                //只有在tls建立成功时才监听
                if (this->connection_status == ConnectionStatus::TLS_CONNECTED)
                {
                    int result = WSAPoll(fds, 1, 100);

                    if (result > 0 && (fds[0].revents & POLLRDNORM)) {
                        int accept_addr_len = sizeof(accept_addr);
                        SOCKET accepted_socket = accept(tcp_listen_socket, (sockaddr*)&accept_addr, &accept_addr_len);
                        if (accepted_socket != INVALID_SOCKET)
                        {
                            if (security_instance)
                            {
                                char client_ip[INET_ADDRSTRLEN];
                                inet_ntop(AF_INET, &(accept_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
                                uint16_t client_port = ntohs(accept_addr.sin_port);

                                if (candidate_ip == client_ip)
                                {
                                    client_socket = accepted_socket;
                                    connect_status = true;
                                    if (cb)
                                        cb(true);
                                    this->connection_status = ConnectionStatus::TCP_ESTABLISHED;
                                }
                            }
                        }
                        else {
                            std::cerr << "fail to accept" << WSAGetLastError() << std::endl;
                        }
                    }
                    else if (result < 0) {
                        std::cerr << "error in WSAPoll" << WSAGetLastError() << std::endl;
                    }
                }
                else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
            }

            closesocket(tcp_listen_socket); });
}

void TcpDriver::startListen(const std::string& address, const std::string& tls_port,
    const std::string& tcp_port, std::function<bool(bool)> tls_callback, std::function<bool(bool)> tcp_callback)
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
    if (!connect_status)
    {
        throw std::runtime_error("connect status is false");
    }
    recv_running = true;
    receive_thread = new std::thread([this, callback = std::move(callback)]()
        {
            while (recv_running)
            {
                msg_parser->delegateRecv(client_socket, callback, this->dcc_cb, this->dre_cb, security_instance);
            }
        });
}
void TcpDriver::closeSocket()
{
    listen_running = false;
    recv_running = false;
    if (connect_status)
    {
        closesocket(client_socket);
    }
    closesocket(tls_listen_socket);
    closesocket(tcp_listen_socket);
    WSACleanup();
    if (tls_listen_thread)
    {
        if (tls_listen_thread->joinable())
            tls_listen_thread->join();
        delete tls_listen_thread;
    }
    if (tcp_listen_thread)
    {
        if (tcp_listen_thread->joinable())
            tcp_listen_thread->join();
        delete tcp_listen_thread;
    }
    if (receive_thread)
    {
        if (receive_thread->joinable())
            receive_thread->join();
        delete receive_thread;
    }
    WSACleanup();
}

void TcpDriver::setSecurityInstance(std::shared_ptr<SecurityInterface> instance)
{
    security_instance = instance;
    msg_builder->setSecurityInstance(instance);
}

void TcpDriver::resetConnection()
{
    if (connect_status)
    {
        closesocket(client_socket);
    }
    recv_running = false;
    client_socket = INVALID_SOCKET;
    candidate_ip.clear();
    client_tls_addr = {};
    client_tcp_addr = {};
    accept_addr = {};
    connect_status = false;
    ignore_one_error = true;
    connection_status = ConnectionStatus::WAITING_TLS;
}