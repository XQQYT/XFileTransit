#ifndef _TCPINTERFACE_H
#define _TCPINTERFACE_H

#include "driver/interface/Network/NetworkInterface.h"

class TcpInterface : public NetworkInterface
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

    virtual ~TcpInterface() = default;
    virtual void setTlsNetworkInfo(const std::string &address, const std::string &tls_port) {}
    virtual void startListen(const std::string &address, const std::string &tls_port, const std::string &tcp_port,
                             std::function<bool(bool)> tls_callback, std::function<bool(bool)> tcp_callback) {}
    virtual void setSecurityInstance(std::shared_ptr<SecurityInterface> instance) { security_instance = instance; }
    virtual void setDealConnectErrorCb(std::function<void(const ConnectError error)> cb) { dce_cb = cb; }
    virtual void setDealRecvErrorCb(std::function<void(const TcpInterface::RecvError error)> cb) { dre_cb = cb; }
    virtual void setDealConnClosedCb(std::function<void()> cb) { dcc_cb = cb; }
    int test = 7;

protected:
    std::shared_ptr<SecurityInterface> security_instance;
    std::function<void(const ConnectError error)> dce_cb;
    std::function<void(const TcpInterface::RecvError error)> dre_cb;
    std::function<void()> dcc_cb;
};

#endif