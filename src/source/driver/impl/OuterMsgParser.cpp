#include "driver/impl/OuterMsgParser.h"
#include "driver/interface/SecurityInterface.h"
#include "driver/interface/PlatformSocket.h"
#include <iostream>
#include <stdexcept>
#include <cstring>

#ifdef _WIN32
// Windows初始化
class WinsockInitializer
{
public:
    WinsockInitializer()
    {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        {
            throw std::runtime_error("WSAStartup failed");
        }
    }

    ~WinsockInitializer()
    {
        WSACleanup();
    }
};

static WinsockInitializer winsockInit;
#endif

void OuterMsgParser::dealRecvError(std::function<void()> dcc_cb,
                                   std::function<void(const NetworkInterface::RecvError error)> dre_cb)
{
    if (dre_cb)
    {
        int error_code = GET_SOCKET_ERROR;
        switch (error_code)
        {
        case SOCKET_ECONNRESET:
            // 对方正常关闭
            if (dcc_cb)
            {
                dcc_cb();
            }
            break;
        case SOCKET_ECONNABORTED:
            dre_cb(NetworkInterface::RecvError::RECV_CONN_ABORTED);
            break;
        case SOCKET_ENOTCONN:
            dre_cb(NetworkInterface::RecvError::RECV_NOT_CONNECTED);
            break;
        case SOCKET_ENETDOWN:
            dre_cb(NetworkInterface::RecvError::RECV_NETWORK_DOWN);
            break;
        case SOCKET_ETIMEDOUT:
            dre_cb(NetworkInterface::RecvError::RECV_TIMED_OUT);
            break;
        case SOCKET_EINTR:
            dre_cb(NetworkInterface::RecvError::RECV_INTERRUPTED);
            break;
        case SOCKET_ESHUTDOWN:
            dre_cb(NetworkInterface::RecvError::RECV_SHUTDOWN);
            break;
        case SOCKET_ENETRESET:
            dre_cb(NetworkInterface::RecvError::RECV_NETWORK_RESET);
            break;
        default:
            std::cerr << "Recv unknown error: " << error_code << std::endl;
            break;
        }
    }
}

void OuterMsgParser::delegateRecv(UnifiedSocket client_socket,
                                  std::function<void(std::unique_ptr<NetworkInterface::UserMsg> parsed_msg)> callback,
                                  std::function<void()> dcc_cb,
                                  std::function<void(const NetworkInterface::RecvError error)> dre_cb,
                                  std::shared_ptr<SecurityInterface> security_instance,
                                  bool &running)
{
    // 设置socket为非阻塞模式
    SOCKET_NONBLOCK(client_socket);

    try
    {
        while (running)
        {
            uint8_t peek_buffer[2];

            // 使用select设置超时
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(client_socket, &readfds);

            struct timeval timeout;
            timeout.tv_sec = 0;
            timeout.tv_usec = 100000; // 100ms超时

#ifdef _WIN32
            int select_result = select(0, &readfds, nullptr, nullptr, &timeout);
#else
            int select_result = select(client_socket + 1, &readfds, nullptr, nullptr, &timeout);
#endif

            if (!running)
            {
                break;
            }

            if (select_result == SOCKET_ERROR_VAL)
            {
                dealRecvError(dcc_cb, dre_cb);
                break;
            }
            else if (select_result == 0)
            {
                // 超时，继续循环
                continue;
            }

            // socket有数据可读
            int peeked = recv(client_socket, reinterpret_cast<char *>(peek_buffer),
                              sizeof(peek_buffer), MSG_PEEK);

            if (peeked > 0)
            {
                if (peek_buffer[0] == 0xAB && peek_buffer[1] == 0xCD)
                {
                    constexpr int HEADER_SIZE = 8;
                    uint8_t buffer[HEADER_SIZE] = {0};
                    uint32_t header_received = 0;

                    while (header_received < HEADER_SIZE && running)
                    {
                        fd_set readfds_inner;
                        FD_ZERO(&readfds_inner);
                        FD_SET(client_socket, &readfds_inner);

                        struct timeval timeout_inner = {1, 0}; // 1秒超时
#ifdef _WIN32
                        if (select(0, &readfds_inner, nullptr, nullptr, &timeout_inner) > 0)
                        {
#else
                        if (select(client_socket + 1, &readfds_inner, nullptr, nullptr, &timeout_inner) > 0)
                        {
#endif
                            int n = recv(client_socket, reinterpret_cast<char *>(buffer + header_received),
                                         HEADER_SIZE - header_received, 0);
                            if (n <= 0)
                            {
                                if (n == 0)
                                {
                                    if (dcc_cb)
                                        dcc_cb(); // 对端关闭
                                }
                                else
                                {
                                    int err = GET_SOCKET_ERROR;
                                    if (err == SOCKET_EWOULDBLOCK)
                                    {
                                        continue;
                                    }
                                    throw std::runtime_error("Header recv error");
                                }
                                break;
                            }
                            header_received += n;
                        }
                    }

                    if (!running)
                        break;
                    if (header_received < HEADER_SIZE)
                        continue;

                    uint32_t payload_length = 0;
                    memcpy(&payload_length, buffer + 3, sizeof(payload_length));
                    payload_length = ntohl(payload_length);

                    uint8_t flag = 0x0;
                    memcpy(&flag, buffer + 7, sizeof(flag));

                    std::vector<uint8_t> receive_msg(payload_length);
                    uint32_t readed_length = 0;

                    while (readed_length < payload_length && running)
                    {
                        fd_set readfds_inner;
                        FD_ZERO(&readfds_inner);
                        FD_SET(client_socket, &readfds_inner);

                        struct timeval timeout_inner = {1, 0}; // 1秒超时
#ifdef _WIN32
                        if (select(0, &readfds_inner, nullptr, nullptr, &timeout_inner) > 0)
                        {
#else
                        if (select(client_socket + 1, &readfds_inner, nullptr, nullptr, &timeout_inner) > 0)
                        {
#endif
                            int read_byte = recv(client_socket, reinterpret_cast<char *>(receive_msg.data() + readed_length),
                                                 payload_length - readed_length, 0);
                            if (read_byte == 0)
                            {
                                if (dcc_cb)
                                    dcc_cb(); // 对端关闭
                                break;
                            }
                            if (read_byte < 0)
                            {
                                int error = GET_SOCKET_ERROR;
                                if (error == SOCKET_EWOULDBLOCK)
                                {
                                    continue; // 非阻塞，重试
                                }
                                throw std::runtime_error("recv error");
                            }
                            readed_length += read_byte;
                        }
                    }

                    if (!running)
                        break;
                    if (readed_length < payload_length)
                        continue;

                    auto parsed = parse(std::move(receive_msg), payload_length, flag);
                    memcpy(&parsed->header, buffer, HEADER_SIZE);
                    std::vector<uint8_t> result_vec;

                    if (flag & static_cast<uint8_t>(NetworkInterface::Flag::IS_ENCRYPT) &&
                        security_instance && security_instance->verifyAndDecrypt(parsed->data, security_instance->getTlsInfo().key.get(), parsed->iv, result_vec, parsed->sha256))
                    {
                        parsed->data.assign(result_vec.begin(), result_vec.end());
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
            else if (peeked == 0)
            {
                // 对方正常关闭
                if (dcc_cb)
                {
                    dcc_cb();
                }
                break;
            }
            else if (peeked == SOCKET_ERROR_VAL)
            {
                int error = GET_SOCKET_ERROR;
                if (error == SOCKET_EWOULDBLOCK)
                {
                    continue; // 非阻塞，重试
                }
                dealRecvError(dcc_cb, dre_cb);
                break;
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "delegateRecv exception: " << e.what() << std::endl;
        dealRecvError(dcc_cb, dre_cb);
    }

    // 恢复阻塞模式
    SOCKET_BLOCK(client_socket);
}

std::unique_ptr<NetworkInterface::UserMsg> OuterMsgParser::parse(std::vector<uint8_t> &&msg, const uint32_t length, const uint8_t flag)
{
    NetworkInterface::UserMsg result;

    size_t offset = 0;

    // 检查标志位
    bool is_encrypt = static_cast<bool>((flag) & static_cast<uint8_t>(NetworkInterface::Flag::IS_ENCRYPT));

    if (is_encrypt)
    {
        // 解析 IV（16字节）
        result.iv.assign(msg.data() + offset, msg.data() + offset + 16);
        offset += 16;

        // 解析 SHA256（32字节）
        result.sha256.assign(msg.data() + offset, msg.data() + offset + 32);
        offset += 32;
    }

    // 解析密文
    size_t cipher_len = length;
    if (is_encrypt)
    {
        cipher_len = length - 16 - 32;
    }

    result.data.assign(msg.data() + offset, msg.data() + offset + cipher_len);
    offset += cipher_len;

    return std::make_unique<NetworkInterface::UserMsg>(std::move(result));
}