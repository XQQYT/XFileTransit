#ifndef _NETWORK_H
#define _NETWORK_H

#include <memory>
#include <functional>
#include "driver/interface/SecurityInterface.h"

class OuterMsgParserInterface;
class NetworkInterface
{
public:
    enum class ConnectError
    {
        CONNECT_REFUSED = 10001,             // WSAECONNREFUSED (10061) - 连接被拒绝
        CONNECT_TIMEOUT = 10002,             // WSAETIMEDOUT (10060) - 连接超时
        CONNECT_HOST_UNREACHABLE = 10003,    // WSAEHOSTUNREACH (10065) - 主机不可达
        CONNECT_NETWORK_UNREACHABLE = 10004, // WSAENETUNREACH (10051) - 网络不可达
        CONNECT_ACCESS_DENIED = 10005,       // WSAEACCES (10013) - 权限被拒绝
        CONNECT_ADDR_IN_USE = 10006,         // WSAEADDRINUSE (10048) - 地址已被使用
        CONNECT_IN_PROGRESS = 10007,         // WSAEWOULDBLOCK/WSAEINPROGRESS - 非阻塞连接中
        CONNECT_ALREADY_CONNECTED = 10008,   // WSAEISCONN (10056) - 已连接
        CONNECT_BAD_ADDRESS = 10009,         // WSAEFAULT (10014) - 地址错误
        CONNECT_INTERRUPTED = 10010,         // WSAEINTR (10004) - 操作被中断
    };
    enum class RecvError
    {
        RECV_CONN_RESET = 20002,    // WSAECONNRESET (10054) - 连接被对端重置
        RECV_CONN_ABORTED = 20003,  // WSAECONNABORTED (10053) - 连接被中止
        RECV_NOT_CONNECTED = 20004, // WSAENOTCONN (10057) - 套接字未连接
        RECV_NETWORK_DOWN = 20005,  // WSAENETDOWN (10050) - 网络子系统故障
        RECV_TIMED_OUT = 20006,     // WSAETIMEDOUT (10060) - 操作超时
        RECV_INTERRUPTED = 20007,   // WSAEINTR (10004) - 操作被中断

        RECV_SHUTDOWN = 20010,      // WSAESHUTDOWN (10058) - 套接字已关闭接收
        RECV_NETWORK_RESET = 20011, // WSAENETRESET (10052) - 网络重置连接
    };

#pragma pack(push, 1)
    struct Header
    {
        uint16_t magic;
        uint8_t version;
        uint32_t length;
        uint8_t flag;
    };
#pragma pack(pop)

    struct UserMsg
    {
        std::vector<uint8_t> iv;
        std::vector<uint8_t> data;
        std::vector<uint8_t> sha256;
        Header header;
    };
    static const uint16_t magic = 0xABCD;
    static const uint8_t version = 0x01;

    enum class Flag : uint8_t
    {
        IS_BINARY = 1 << 0,
        IS_ENCRYPT = 1 << 1
    };

    friend Flag operator|(Flag lhs, Flag rhs)
    {
        return static_cast<Flag>(
            static_cast<std::underlying_type_t<Flag>>(lhs) |
            static_cast<std::underlying_type_t<Flag>>(rhs));
    }

    friend bool operator&(Flag lhs, Flag rhs)
    {
        return static_cast<bool>(
            static_cast<std::underlying_type_t<Flag>>(lhs) &
            static_cast<std::underlying_type_t<Flag>>(rhs));
    }

public:
    NetworkInterface() = default;
    NetworkInterface(const NetworkInterface &obj) = delete;
    NetworkInterface(NetworkInterface &&obj) = delete;
    NetworkInterface &operator=(NetworkInterface &other) = delete;
    NetworkInterface &operator=(NetworkInterface &&other) = delete;
    virtual ~NetworkInterface() = default;
    virtual void initTlsSocket(const std::string &address, const std::string &tls_port) = 0;
    virtual void initTcpSocket(const std::string &address, const std::string &tcp_port) = 0;
    virtual void connectTo(std::function<void(bool)> callback = nullptr) = 0;
    virtual void startListen(const std::string &address, const std::string &tls_port, const std::string &tcp_port,
                             std::function<bool(bool)> tls_callback, std::function<bool(bool)> tcp_callback) = 0;
    virtual void startTlsListen(const std::string &address, const std::string &tls_port, std::function<bool(bool)> tls_callback) = 0;
    virtual void startTcpListen(const std::string &address, const std::string &tcp_port, std::function<bool(bool)> tcp_callback) = 0;
    virtual void sendMsg(const std::string &msg) = 0;
    virtual void recvMsg(std::function<void(std::unique_ptr<UserMsg>)> callback) = 0;
    virtual void closeSocket() = 0;
    virtual void resetConnection() = 0;
    virtual void setSecurityInstance(std::shared_ptr<SecurityInterface> instance) { security_instance = instance; }
    virtual void setDealConnectErrorCb(std::function<void(const ConnectError error)> cb) { dce_cb = cb; }
    virtual void setDealRecvErrorCb(std::function<void(const NetworkInterface::RecvError error)> cb) { dre_cb = cb; }
    virtual void setDealConnClosedCb(std::function<void()> cb) { dcc_cb = cb; }

protected:
    std::shared_ptr<SecurityInterface> security_instance;
    std::function<void(const ConnectError error)> dce_cb;
    std::function<void(const NetworkInterface::RecvError error)> dre_cb;
    std::function<void()> dcc_cb;
};

#endif