#ifndef _TCPDRIVER_H
#define _TCPDRIVER_H

#include "driver/interface/NetworkInterface.h"
#include "driver/interface/MsgBuilderInterface.h"
#include <thread>
#include <atomic>

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

class TcpDriver : public NetworkInterface
{
public:
    TcpDriver();
    ~TcpDriver();
    void initSocket(const std::string& address, const std::string& port) override;
    void connectTo(std::function<void(bool)> callback = nullptr) override;
    void sendMsg(std::string msg) override;
    void recvMsg(std::function<void(std::vector<uint8_t>, bool)> callback);
    void closeSocket() override;
    void setSecurityInstance(std::shared_ptr<SecurityInterface> instance) override;
private:
    WSADATA wsa_data;
    SOCKET tcp_socket;
    std::unique_ptr<MsgBuilderInterface> msg_builder;
    sockaddr_in addr;
    SecurityInterface::TlsInfo tls_info;
    std::thread* receive_thread;
    std::atomic<bool> runing{ false };
    bool connect_status;
};

#endif //_TCPDRIVER_H