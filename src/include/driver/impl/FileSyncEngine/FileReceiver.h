#ifndef FILERECEIVER_H
#define FILERECEIVER_H

#include "driver/interface/FileSyncEngine/FileReceiverInterface.h"
#include "driver/interface/OuterMsgParserInterface.h"
#include <thread>
#include <vector>

class FileReceiver : public FileReceiverInterface
{
public:
    using FileReceiverInterface::FileReceiverInterface;
    bool initialize() override;
    void start(std::function<void(SOCKET)> accept_cb,
        std::function<void(SOCKET socket, std::unique_ptr<NetworkInterface::UserMsg>)> msg_cb) override;
    void stop() override;
private:
    SOCKET createListenSocket(const std::string& address, const std::string& port);
    void closeReceiver();
private:
    SOCKET listen_socket;
    sockaddr_in accept_addr;
    std::thread* tcp_listen_thread;
    std::vector<SOCKET> receive_sockets;
    std::vector<std::thread*> receive_threads;
    std::unique_ptr<OuterMsgParserInterface> outer_parser;
};

#endif