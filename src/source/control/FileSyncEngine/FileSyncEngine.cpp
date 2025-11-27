#include "control/FileSyncEngine/FileSyncEngine.h"
#include "driver/impl/OuterMsgBuilder.h"
#include "driver/impl/OuterMsgParser.h"
#include "driver/impl/FileSyncEngine/FileSender.h"
#include "driver/impl/FileSyncEngine/FileReceiver.h"
#include "control/EventBusManager.h"

FileSyncEngine::FileSyncEngine():
outer_msg_builder(std::make_shared<OuterMsgBuilder>()),
outer_msg_parser(std::make_shared<OuterMsgParser>()),
cv(std::make_shared<std::condition_variable>())
{
    EventBusManager::instance().subscribe("/file/have_file_to_send", std::bind(
        &FileSyncEngine::onHaveFileToSend,
        this,
        std::placeholders::_1));
}

void FileSyncEngine::onHaveFileToSend(uint32_t id, std::string path)
{
    std::lock_guard<std::mutex> lock(mtx);
    pending_send_files.push({id, std::move(path)});
    cv->notify_one();
}

void FileSyncEngine::start(std::string address, std::string recv_port, 
                          std::shared_ptr<SecurityInterface> instance) {
    for(int i = 0; i < sender_num; ++i)
    {
        file_senders.emplace_back(address, recv_port, instance);
        auto& sender = file_senders.back();
        if (sender.initialize())
        {
            sender.setCondition(this->cv);
            sender.start(nullptr);
        }
        else 
        {
            file_senders.pop_back();
        }
    }

    file_receiver = std::make_unique<FileReceiver>(address, recv_port, instance);
    if(file_receiver->initialize())
    {
        file_receiver->start(nullptr);
    }
}

void FileSyncEngine::stop()
{
    //关闭线程
    for(auto& i : file_senders)
    {
        i.stop();
    }
    file_receiver->stop();
    //销毁资源
    file_senders.pop_back();
    file_receiver.release();
}
