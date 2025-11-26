#include "driver/impl/TcpDriver.h"
#include "driver/impl/MsgBuilder.h"
#include <iostream>
#include <memory.h>
#include <iomanip>

TcpDriver::TcpDriver() : msg_builder(std::make_unique<MsgBuilder>(security_instance)),
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
    if(this->dce_cb)
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

void TcpDriver::dealRecvError()
{
    if(this->dre_cb)
    {
        int error_code = WSAGetLastError();
        switch (error_code) {
        case WSAECONNRESET:
            //对方正常关闭
            if(this->dcc_cb)
            {
                this->dcc_cb();
            }
            break;
        case WSAECONNABORTED:
            this->dre_cb(RecvError::RECV_CONN_ABORTED);
            break;
        case WSAENOTCONN:
            this->dre_cb(RecvError::RECV_NOT_CONNECTED);
            break;
        case WSAENETDOWN:
            this->dre_cb(RecvError::RECV_NETWORK_DOWN);
            break;
        case WSAETIMEDOUT:
            this->dre_cb(RecvError::RECV_TIMED_OUT);
            break;
        case WSAEINTR:
            this->dre_cb(RecvError::RECV_INTERRUPTED);
            break;
        case WSAESHUTDOWN:
            this->dre_cb(RecvError::RECV_SHUTDOWN);
            break;
        case WSAENETRESET:
            this->dre_cb(RecvError::RECV_NETWORK_RESET);
            break;
        default:
            std::cerr << "Recv unknown error" << std::endl;
            break;
        }        
    }
}

void TcpDriver::connectTo(std::function<void(bool)> callback)
{
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
    std::unique_ptr<MsgBuilderInterface::UserMsg> ready_to_send_msg = std::move(msg_builder->buildMsg(msg));

    size_t final_msg_length = ready_to_send_msg->msg->size();
    size_t sended_length = 0;

    while (sended_length < final_msg_length)
    {
        int ret = send(client_socket, reinterpret_cast<const char*>(ready_to_send_msg->msg->data() + sended_length),
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
    tls_listen_socket =  createListenSocket(address, tls_port);

    tls_listen_thread = new std::thread([this, cb = std::move(tls_callback)]()
        {
            WSAPOLLFD fds[1];
            fds[0].fd = tls_listen_socket;
            fds[0].events = POLLRDNORM;

            while (this->listen_running)
            {
                //只有在等待TLS请求时才接收
                if(this->connection_status == ConnectionStatus::WAITING_TLS)
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
                                    if(cb)
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
    tcp_listen_socket =  createListenSocket(address, tcp_port);

    tcp_listen_thread = new std::thread([this, cb = std::move(tcp_callback)]()
        {
            WSAPOLLFD fds[1];
            fds[0].fd = tcp_listen_socket;
            fds[0].events = POLLRDNORM;

            while (this->listen_running)
            {
                //只有在tls建立成功时才监听
                if(this->connection_status == ConnectionStatus::TLS_CONNECTED)
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
                                    if(cb)
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
    const std::string& tcp_port,std::function<bool(bool)> tls_callback, std::function<bool(bool)> tcp_callback)
{
    listen_running = true;
    if(security_instance)
    {
        startTlsListen(address, tls_port, tls_callback);
    }
    startTcpListen(address, tcp_port, tcp_callback);
}

NetworkInterface::ParsedMsg parseMsgPayload(const uint8_t* full_msg, const uint32_t length, const uint8_t flag)
{
    NetworkInterface::ParsedMsg result;

    size_t offset = 0;

    // 检查标志位
    bool is_encrypt = static_cast<bool>((flag) & static_cast<uint8_t>(MsgBuilderInterface::Flag::IS_ENCRYPT));

    if (is_encrypt)
    {
        // 解析 IV（16字节）
        result.iv.assign(full_msg + offset, full_msg + offset + 16);
        offset += 16;

        // 解析 SHA256（32字节）
        result.sha256.assign(full_msg + offset, full_msg + offset + 32);
        offset += 32;
    }

    // 解析密文
    size_t cipher_len = length;
    if (is_encrypt)
    {
        cipher_len = length - 16 - 32;
    }

    result.data.assign(full_msg + offset, full_msg + offset + cipher_len);
    offset += cipher_len;

    return result;
}

void TcpDriver::recvMsg(std::function<void(ParsedMsg&& parsed_msg)> callback)
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
                uint8_t peek_buffer[2];
                int peeked = recv(client_socket, reinterpret_cast<char*>(peek_buffer), sizeof(peek_buffer), MSG_PEEK);
                if (peeked > 0) {
                    if (peek_buffer[0] == 0xAB && peek_buffer[1] == 0xCD)
                    {
                        constexpr int HEADER_SIZE = 8;
                        uint8_t buffer[HEADER_SIZE] = { 0 };
                        uint32_t header_received = 0;
                        while (header_received < HEADER_SIZE) {
                            int n = recv(client_socket, reinterpret_cast<char*>(buffer + header_received),
                                HEADER_SIZE - header_received, 0);
                            if (n <= 0) throw std::runtime_error("Header recv error");
                            header_received += n;
                        }

                        uint32_t payload_length = 0;
                        memcpy(&payload_length, buffer + 3, sizeof(payload_length));
                        payload_length = ntohl(payload_length);

                        uint8_t flag = 0x0;
                        memcpy(&flag, buffer + 7, sizeof(flag));

                        uint8_t* receive_msg = new uint8_t[payload_length];
                        uint32_t readed_length = 0;

                        while (readed_length < payload_length) {
                            int read_byte = recv(client_socket, reinterpret_cast<char*>(receive_msg + readed_length),
                                payload_length - readed_length, 0);
                            if (read_byte == 0) {
                                throw std::runtime_error("peer closed");
                            }
                            if (read_byte < 0) {
                                throw std::runtime_error("recv error");
                            }
                            readed_length += read_byte;
                            std::cout << readed_length << " / " << payload_length << std::endl;
                        }

                        printHex(std::vector<uint8_t>(buffer, buffer + 8));
                        auto parsed = parseMsgPayload(receive_msg, payload_length, flag);
                        memcpy(&parsed.header, buffer, HEADER_SIZE);
                        std::vector<uint8_t> result_vec;
                        result_vec.reserve(parsed.data.size());
                        if (flag & static_cast<uint8_t>(MsgBuilderInterface::Flag::IS_ENCRYPT) &&
                            security_instance->verifyAndDecrypt(parsed.data, security_instance->getTlsInfo().key.get(), parsed.iv, result_vec, parsed.sha256))
                        {
                            parsed.data.assign(result_vec.begin(), result_vec.end());
                            callback(std::move(parsed));
                        }
                        else
                        {
                            callback(std::move(parsed));
                        }
                    }
                    else
                    {
                        char dump_buffer[4];
                        recv(client_socket, dump_buffer, sizeof(dump_buffer), 0);
                    }

                }//对方正常关闭
                else if (peeked == 0) {
                    if(this->dcc_cb)
                    {
                        this->dcc_cb();
                    }
                }else if (peeked == SOCKET_ERROR) {//错误处理
                    dealRecvError();
                }
            } });
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

    connection_status = ConnectionStatus::WAITING_TLS;
}