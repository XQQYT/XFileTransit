#ifndef FILERECEIVER_H
#define FILERECEIVER_H

#include "driver/interface/FileSyncEngine/FileReceiverInterface.h"
#include "driver/interface/SecurityInterface.h"
#include "driver/interface/PlatformSocket.h"
#include <functional>
#include <memory>
#include <vector>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <atomic>

class OuterMsgParser;

class FileReceiver : public FileReceiverInterface
{
public:
    FileReceiver();
    ~FileReceiver();

    bool initialize(const std::string &addr, const std::string &p,
                    std::shared_ptr<SecurityInterface> inst) override;

    void start(std::function<void(UnifiedSocket)> accept_cb,
               std::function<void(UnifiedSocket socket,
                                  std::unique_ptr<NetworkInterface::UserMsg>)>
                   msg_cb) override;

    void closeReceiver() override;
    void removeSocket(UnifiedSocket socket) override;
    void stop() override;

    UnifiedSocket createListenSocket(const std::string &address, const std::string &port);

private:
#ifdef _WIN32
    // Windows相关方法
    void handleAccept(std::vector<WSAPOLLFD> &poll_fds,
                      std::unordered_map<SOCKET, size_t> &socket_to_index,
                      std::function<void(UnifiedSocket)> &accept_cb,
                      std::function<void(UnifiedSocket, std::unique_ptr<NetworkInterface::UserMsg>)> &msg_cb);
    void handleSocketEvent(SOCKET socket,
                           std::function<void(UnifiedSocket, std::unique_ptr<NetworkInterface::UserMsg>)> &msg_cb);
    void cleanupClosedSockets(std::vector<WSAPOLLFD> &poll_fds,
                              std::unordered_map<SOCKET, size_t> &socket_to_index);
#else
    // Linux相关方法
    void handleAcceptEpoll(int epoll_fd,
                           std::unordered_map<int, std::shared_ptr<std::thread>> &socket_threads,
                           std::function<void(UnifiedSocket)> &accept_cb,
                           std::function<void(UnifiedSocket, std::unique_ptr<NetworkInterface::UserMsg>)> &msg_cb);
#endif

    void startReceiveThread(UnifiedSocket socket,
                            std::function<void(UnifiedSocket, std::unique_ptr<NetworkInterface::UserMsg>)> &msg_cb);
    void receiveThreadFunction(UnifiedSocket socket,
                               std::function<void(UnifiedSocket, std::unique_ptr<NetworkInterface::UserMsg>)> &msg_cb);
    bool processSocketData(UnifiedSocket socket,
                           std::function<void(UnifiedSocket, std::unique_ptr<NetworkInterface::UserMsg>)> &msg_cb);
    bool processProtocolMessage(UnifiedSocket socket,
                                std::function<void(UnifiedSocket, std::unique_ptr<NetworkInterface::UserMsg>)> &msg_cb);

private:
    UnifiedSocket listen_socket;
    std::string address;
    std::string port;
    std::atomic<bool> running{false};

    std::unique_ptr<OuterMsgParser> outer_parser;
    std::shared_ptr<SecurityInterface> security_instance;

    std::unique_ptr<std::thread> tcp_listen_thread;
    std::vector<std::shared_ptr<std::thread>> receive_threads;
    std::vector<UnifiedSocket> receive_sockets;

    std::mutex sockets_mutex;
    std::mutex threads_mutex;
};

#endif // FILERECEIVER_H