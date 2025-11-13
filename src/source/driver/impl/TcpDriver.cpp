#include "driver/impl/TcpDriver.h"
#include "driver/impl/MsgBuilder.h"
#include <iostream>
#include <memory.h>
#include <iomanip>

TcpDriver::TcpDriver() : msg_builder(std::make_unique<MsgBuilder>(security_instance)),
client_addr({}),
listen_addr({}),
receive_thread(nullptr),
listen_thread(nullptr)
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

void TcpDriver::initSocket(const std::string& address, const std::string& port)
{
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(std::stoi(port));
    inet_pton(AF_INET, address.c_str(), &client_addr.sin_addr);
    this->client_address = address;
    this->client_port = port;
}

void TcpDriver::connectTo(std::function<void(bool)> callback)
{
    int ret = connect(client_socket, (sockaddr*)&client_addr, sizeof(client_addr));
    if (ret == SOCKET_ERROR) {
        int error_code = WSAGetLastError();

        switch (error_code) {
        case WSAECONNREFUSED:
            std::cerr << "Connection refused (No service listening on target port)" << std::endl;
            break;
        case WSAETIMEDOUT:
            std::cerr << "Connection timed out" << std::endl;
            break;
        case WSAEHOSTUNREACH:
            std::cerr << "Host unreachable" << std::endl;
            break;
        case WSAENETUNREACH:
            std::cerr << "Network unreachable" << std::endl;
            break;
        case WSAEADDRINUSE:
            std::cerr << "Address already in use" << std::endl;
            break;
        case WSAEINPROGRESS:
            std::cerr << "Non-blocking socket connection in progress" << std::endl;
            break;
        case WSAEACCES:
            std::cerr << "Permission denied" << std::endl;
            break;
        case WSAEAFNOSUPPORT:
            std::cerr << "Address family not supported" << std::endl;
            break;
        case WSAECONNABORTED:
            std::cerr << "Connection aborted" << std::endl;
            break;
        case WSAECONNRESET:
            std::cerr << "Connection reset by peer" << std::endl;
            break;
        case WSAENOTCONN:
            std::cerr << "Socket is not connected" << std::endl;
            break;
        default:
            std::cerr << "Unknown error" << std::endl;
            break;
        }
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

        ret = connect(client_socket, (sockaddr*)&client_addr, sizeof(client_addr));
    }

    if (callback)
    {
        std::cout << ret << std::endl;
        callback(!ret);
    }
    connect_status = !ret;
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
void TcpDriver::startListen(const std::string& address, const std::string& port, std::function<bool(bool)> callback)
{
    runing = true;

    listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_socket == INVALID_SOCKET)
    {
        std::cerr << "create listen socket failed" << WSAGetLastError() << std::endl;
        return;
    }

    listen_addr.sin_family = AF_INET;
    listen_addr.sin_port = htons(std::stoi(port));
    inet_pton(AF_INET, address.c_str(), &listen_addr.sin_addr);

    if (bind(listen_socket, (sockaddr*)&listen_addr, sizeof(listen_addr)) == SOCKET_ERROR)
    {
        std::cerr << "fail to bind" << WSAGetLastError() << std::endl;
        closesocket(listen_socket);
        return;
    }

    if (listen(listen_socket, 5) == SOCKET_ERROR)
    {
        std::cerr << "fail to listen" << WSAGetLastError() << std::endl;
        closesocket(listen_socket);
        return;
    }

    listen_thread = new std::thread([this, cb = std::move(callback)]()
        {
            WSAPOLLFD fds[1];
            fds[0].fd = listen_socket;
            fds[0].events = POLLRDNORM;

            while (runing)
            {
                int result = WSAPoll(fds, 1, 100);

                if (result > 0 && (fds[0].revents & POLLRDNORM)) {
                    int accept_addr_len = sizeof(accept_addr);
                    SOCKET accepted_socket = accept(listen_socket, (sockaddr*)&accept_addr, &accept_addr_len);
                    if (accepted_socket != INVALID_SOCKET)
                    {
                        if (security_instance)
                        {
                            if (candidate_ip.empty())
                            {
                                security_instance->dealTlsRequest(accepted_socket, [this, accepted_socket](bool ret, SecurityInterface::TlsInfo info) {
                                    if (ret)
                                    {
                                        if (accepted_socket != INVALID_SOCKET) {
                                            char client_ip[INET_ADDRSTRLEN];
                                            inet_ntop(AF_INET, &(accept_addr.sin_addr), client_ip, INET_ADDRSTRLEN);

                                            candidate_ip.assign(client_ip);
                                        }
                                        security_instance->setTlsInfo(info);
                                    }
                                    });
                            }
                            else
                            {
                                char client_ip[INET_ADDRSTRLEN];
                                inet_ntop(AF_INET, &(accept_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
                                uint16_t client_port = ntohs(accept_addr.sin_port);

                                if (candidate_ip == client_ip)
                                {
                                    client_socket = accepted_socket;
                                    connect_status = true;
                                    cb(true);
                                }
                            }
                        }
                        else
                        {
                            client_socket = accepted_socket;
                            connect_status = true;
                            cb(true);
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

            closesocket(listen_socket); });
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
    runing = true;
    receive_thread = new std::thread([this, callback = std::move(callback)]()
        {
            while (runing)
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

                }
                else if (peeked == 0) {
                    break;
                }
                else {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        continue;
                    }
                    else if (errno == EINTR) {
                        continue;
                    }
                    else {
                        perror("recv error");
                        break;
                    }
                }

            } });
}

void TcpDriver::closeSocket()
{
    runing = false;
    if (connect_status)
    {
        closesocket(client_socket);
    }
    closesocket(listen_socket);
    WSACleanup();
    if (listen_thread)
    {
        if (listen_thread->joinable())
            listen_thread->join();
        delete listen_thread;
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