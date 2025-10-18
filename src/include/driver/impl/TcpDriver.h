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
    void sendMsg(const std::string& msg) override;
    void startListen(const std::string& address, const std::string& port, std::function<bool(bool)> callback) override;
    void recvMsg(std::function<void(ParsedMsg&& parsed_msg)> callback) override;
    void closeSocket() override;
    void setSecurityInstance(std::shared_ptr<SecurityInterface> instance) override;

private:
    WSADATA wsa_data;
    SOCKET client_socket = INVALID_SOCKET;
    SOCKET listen_socket = INVALID_SOCKET;
    SOCKET candidate_socket = INVALID_SOCKET;
    std::unique_ptr<MsgBuilderInterface> msg_builder;
    sockaddr_in client_addr;
    sockaddr_in listen_addr;
    std::thread* receive_thread;
    std::thread* listen_thread;
    std::atomic<bool> runing{ false };
    std::atomic<bool> connect_status{ false };
};

#endif //_TCPDRIVER_H