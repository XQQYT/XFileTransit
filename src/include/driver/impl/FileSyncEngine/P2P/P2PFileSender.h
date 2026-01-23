#ifndef P2PFILESENDER_H
#define P2PFILESENDER_H

#include "driver/interface/FileSyncEngine/FileSenderInterface.h"
#include "driver/interface/FileSyncEngine/FileMsgBuilderInterface.h"
#include "driver/interface/Network/P2PInterface.h"
#include <mutex>
#include <thread>
#include <chrono>
#include <memory>
#include <condition_variable>

class P2PFileSender : public FileSenderInterface
{
public:
    using FileSenderInterface::FileSenderInterface;
    void start(std::function<std::optional<std::pair<uint32_t, std::string>>()> get_task_cb) override;
    void stop() override;
    ~P2PFileSender();
    void setP2PInstance(std::shared_ptr<P2PInterface> inst)
    {
        p2p_inst = inst;
        sender_label = p2p_inst->getOneReceiveDc();
    }

private:
    void sendMsg(std::vector<uint8_t> &&msg, bool is_binary);

private:
    std::shared_ptr<P2PInterface> p2p_inst;

    std::mutex mtx;
    std::condition_variable cv;
    std::thread *send_thread{nullptr};
    std::unique_ptr<FileMsgBuilderInterface> file_msg_builder;

    uint32_t bytes_sent{0};
    std::chrono::steady_clock::time_point start_time_point;
    std::chrono::steady_clock::time_point end_time_point;

    std::string sender_label;
};

#endif // P2PFILESENDER_H