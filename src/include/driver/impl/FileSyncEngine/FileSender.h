#include "driver/interface/FileSyncEngine/FileSenderInterface.h"
#include "driver/interface/FileSyncEngine/FileMsgBuilderInterface.h"
#include <mutex>
#include <thread>

class FileSender : public FileSenderInterface
{
public:
    using FileSenderInterface::FileSenderInterface;
    bool initialize() override;
    void start(std::function<std::pair<uint32_t, std::string>()> get_task_cb) override;
    void stop() override;
private:
    sockaddr_in client_tcp_addr;
    SOCKET client_socket = INVALID_SOCKET;
    WSADATA wsa_data;

    std::mutex mtx;
    std::thread* send_thread{ nullptr };

    std::unique_ptr<FileMsgBuilderInterface> file_msg_builder;
};