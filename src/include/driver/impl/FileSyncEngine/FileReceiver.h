#ifndef FILERECEIVER_H
#define FILERECEIVER_H

#include "driver/interface/FileSyncEngine/FileReceiverInterface.h"
#include "driver/interface/OuterMsgParserInterface.h"
#include "driver/interface/PlatformSocket.h"
#include <thread>
#include <vector>
#include <mutex>
#include <memory>

class FileReceiver : public FileReceiverInterface
{
public:
    using FileReceiverInterface::FileReceiverInterface;

    bool initialize() override;
    void start(std::function<void(UnifiedSocket)> accept_cb,
               std::function<void(UnifiedSocket socket, std::unique_ptr<NetworkInterface::UserMsg>)> msg_cb) override;
    void stop() override;

    virtual ~FileReceiver()
    {
        stop();
    }

private:
    UnifiedSocket createListenSocket(const std::string &address, const std::string &port);
    void closeReceiver();
    void removeSocket(UnifiedSocket socket);

private:
    UnifiedSocket listen_socket;
    sockaddr_in accept_addr;
    std::unique_ptr<std::thread> tcp_listen_thread;
    std::vector<UnifiedSocket> receive_sockets;
    std::vector<std::shared_ptr<std::thread>> receive_threads;
    std::unique_ptr<OuterMsgParserInterface> outer_parser;

    std::mutex sockets_mutex; // 保护receive_sockets
    std::mutex threads_mutex; // 保护receive_threads

protected:
    using FileReceiverInterface::address;
    using FileReceiverInterface::port;
    using FileReceiverInterface::running;
    using FileReceiverInterface::security_instance;
};

#endif // FILERECEIVER_H