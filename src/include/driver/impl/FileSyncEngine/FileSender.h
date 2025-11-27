#include "driver/interface/FileSyncEngine/FileSenderInterface.h"
#include <mutex>

class FileSender : public FileSenderInterface
{
public:
    using FileSenderInterface::FileSenderInterface;
    bool initialize() override;
    void start(std::function<std::pair<uint32_t,std::string>()> get_task_cb) override;
    void stop() override;
private:
    sockaddr_in client_tcp_addr;
    SOCKET client_socket = INVALID_SOCKET;

    std::mutex mtx;
};