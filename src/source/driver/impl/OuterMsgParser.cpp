#include "driver/impl/OuterMsgParser.h"
#include "driver/interface/SecurityInterface.h"
#include "driver/interface/PlatformSocket.h"
#include "common/DebugOutputer.h"
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
                                   std::function<void(const TcpInterface::RecvError error)> dre_cb)
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
            dre_cb(TcpInterface::RecvError::RECV_CONN_ABORTED);
            break;
        case SOCKET_ENOTCONN:
            dre_cb(TcpInterface::RecvError::RECV_NOT_CONNECTED);
            break;
        case SOCKET_ENETDOWN:
            dre_cb(TcpInterface::RecvError::RECV_NETWORK_DOWN);
            break;
        case SOCKET_ETIMEDOUT:
            dre_cb(TcpInterface::RecvError::RECV_TIMED_OUT);
            break;
        case SOCKET_EINTR:
            dre_cb(TcpInterface::RecvError::RECV_INTERRUPTED);
            break;
        case SOCKET_ESHUTDOWN:
            dre_cb(TcpInterface::RecvError::RECV_SHUTDOWN);
            break;
        case SOCKET_ENETRESET:
            dre_cb(TcpInterface::RecvError::RECV_NETWORK_RESET);
            break;
        default:
            LOG_ERROR("Recv unknown error: " << error_code);
            break;
        }
    }
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