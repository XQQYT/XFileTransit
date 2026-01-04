#ifndef FILESENDER_H
#define FILESENDER_H

#include "driver/interface/FileSyncEngine/FileSenderInterface.h"
#include "driver/interface/FileSyncEngine/FileMsgBuilderInterface.h"
#include "driver/interface/PlatformSocket.h"
#include <mutex>
#include <thread>
#include <chrono>
#include <memory>
#include <condition_variable>

class FileSender : public FileSenderInterface
{
public:
    using FileSenderInterface::FileSenderInterface;
    bool initialize(const std::string &addr, const std::string &p, std::shared_ptr<SecurityInterface> inst) override;
    void start(std::function<std::optional<std::pair<uint32_t, std::string>>()> get_task_cb) override;
    void stop() override;
    ~FileSender() override;

private:
    void sendMsg(std::vector<uint8_t> &&msg, bool is_binary);

private:
    sockaddr_in client_tcp_addr;
    UnifiedSocket client_socket = INVALID_SOCKET_VAL;

#ifdef _WIN32
    WSADATA wsa_data;
#endif

    std::mutex mtx;
    std::condition_variable cv;
    std::thread *send_thread{nullptr};
    std::unique_ptr<FileMsgBuilderInterface> file_msg_builder;

    uint32_t bytes_sent{0};
    std::chrono::steady_clock::time_point start_time_point;
    std::chrono::steady_clock::time_point end_time_point;
};

#endif // FILESENDER_H