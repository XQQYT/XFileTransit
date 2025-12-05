#include "driver/interface/FileSyncEngine/FileSenderInterface.h"
#include "driver/interface/FileSyncEngine/FileMsgBuilderInterface.h"
#include <mutex>
#include <thread>
#include <chrono>

class FileSender : public FileSenderInterface
{
public:
    using FileSenderInterface::FileSenderInterface;
    bool initialize() override;
    void start(std::function<std::optional<std::pair<uint32_t, std::string>>()> get_task_cb) override;
    void stop() override;
    ~FileSender();
private:
    void sendMsg(std::vector<uint8_t>&& msg, bool is_binary);
private:
    sockaddr_in client_tcp_addr;
    SOCKET client_socket = INVALID_SOCKET;
    WSADATA wsa_data;

    std::mutex mtx;
    std::thread* send_thread{ nullptr };
    std::unique_ptr<FileMsgBuilderInterface> file_msg_builder;

    uint32_t bytes_sent{ 0 };
    std::chrono::steady_clock::time_point start_time_point;
    std::chrono::steady_clock::time_point end_time_point;
};