#ifndef _TCPDRIVER_H
#define _TCPDRIVER_H

#include "driver/interface/Network/TcpInterface.h"
#include "driver/interface/OuterMsgBuilderInterface.h"
#include "driver/interface/OuterMsgParserInterface.h"
#include <thread>
#include <atomic>

class TcpDriver : public TcpInterface
{
public:
    TcpDriver();
    ~TcpDriver();

    virtual void startListen(const std::string &address, const std::string &port, std::function<bool(bool)> callback) {}

    void setTlsNetworkInfo(const std::string &address, const std::string &tls_port) override;
    void connect(const std::string &address, const std::string &port, std::function<void(bool)> callback = nullptr) override;
    void sendMsg(const std::string &msg) override;
    // 设置安全实例才会开启tls监听
    void startListen(const std::string &address, const std::string &tls_port, const std::string &tcp_port,
                     std::function<bool(bool)> tls_callback, std::function<bool(bool)> tcp_callback) override;
    void startTlsListen(const std::string &address, const std::string &tls_port, std::function<bool(bool)> tls_callback);
    void startTcpListen(const std::string &address, const std::string &tcp_port, std::function<bool(bool)> tcp_callback);
    void recvMsg(std::function<void(std::unique_ptr<NetworkInterface::UserMsg>)> callback) override;
    void closeSocket() override;
    void setSecurityInstance(std::shared_ptr<SecurityInterface> instance) override;
    void resetConnection() override;

private:
    enum class ConnectionStatus
    {
        WAITING_TLS,
        TLS_CONNECTED,
        TCP_ESTABLISHED
    } connection_status;

    UnifiedSocket createListenSocket(const std::string &address, const std::string &port);
    void dealConnectError();

private:
#ifdef _WIN32
    WSADATA wsa_data;
#endif
    UnifiedSocket client_socket = INVALID_SOCKET_VAL;
    UnifiedSocket tls_listen_socket = INVALID_SOCKET_VAL;
    UnifiedSocket tcp_listen_socket = INVALID_SOCKET_VAL;

    UnifiedSocket tls_wakeup_event;
    UnifiedSocket tcp_wakeup_event;
    UnifiedSocket recv_wakeup_event;

    std::string candidate_ip;
    std::unique_ptr<OuterMsgBuilderInterface> msg_builder;
    std::unique_ptr<OuterMsgParserInterface> msg_parser;

    sockaddr_in client_tls_addr;
    sockaddr_in client_tcp_addr;
    sockaddr_in accept_addr;

    std::thread *receive_thread;
    std::thread *tls_listen_thread;
    std::thread *tcp_listen_thread;

    bool ignore_one_error{false};

    std::atomic<bool> listen_running{false};
    bool recv_running{false};
    std::atomic<bool> connect_status{false};
};

#endif //_TCPDRIVER_H