#include "control/FileSyncEngine/FileSyncEngine.h"
#include "driver/impl/OuterMsgBuilder.h"
#include "driver/impl/OuterMsgParser.h"
#include "driver/impl/FileSyncEngine/FileSender.h"
#include "driver/impl/FileSyncEngine/FileReceiver.h"
#include "control/EventBusManager.h"
#include <iostream>

FileSyncEngine::FileSyncEngine() :
    outer_msg_builder(std::make_shared<OuterMsgBuilder>()),
    outer_msg_parser(std::make_shared<OuterMsgParser>()),
    cv(std::make_shared<std::condition_variable>())
{
    EventBusManager::instance().subscribe("/file/initialize_FileSyncCore", std::bind(
        &FileSyncEngine::start,
        this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3));
    EventBusManager::instance().subscribe("/file/close_FileSyncCore", std::bind(
        &FileSyncEngine::stop,
        this));
    EventBusManager::instance().subscribe("/file/have_file_to_send", std::bind(
        &FileSyncEngine::onHaveFileToSend,
        this,
        std::placeholders::_1,
        std::placeholders::_2));
}

void FileSyncEngine::onHaveFileToSend(uint32_t id, std::string path)
{
    std::lock_guard<std::mutex> lock(mtx);
    pending_send_files.push({ id, std::move(path) });
    cv->notify_one();
}

std::pair<uint32_t, std::string> FileSyncEngine::getPendingFile()
{
    std::lock_guard<std::mutex> lock(mtx);
    if (pending_send_files.empty()) {
        return { 0, "" };
    }
    auto file = pending_send_files.front();
    pending_send_files.pop();
    return file;
}

void FileSyncEngine::receiveProgress(uint32_t id, float progress)
{

}

void FileSyncEngine::start(std::string address, std::string recv_port,
    std::shared_ptr<SecurityInterface> instance)
{
    std::cout << "FileSyncCore start" << std::endl;

    //初始化receiver
    file_receiver = std::make_unique<FileReceiver>("0.0.0.0", recv_port, instance);

    if (file_receiver->initialize())
    {
        file_receiver->start(std::bind(&FileSyncEngine::receiveProgress, this, std::placeholders::_1, std::placeholders::_2));
    }

    //初始化sender
    std::vector<std::shared_ptr<FileSender>> initialized_senders;
    for (int i = 0; i < sender_num; ++i)
    {
        auto sender = std::make_shared<FileSender>(address, recv_port, instance);
        if (sender->initialize())
        {
            sender->setCondition(this->cv);
            initialized_senders.push_back(sender);
        }
    }

    for (auto& sender : initialized_senders)
    {
        sender->start(std::bind(&FileSyncEngine::getPendingFile, this));
        file_senders.push_back(sender);
    }
    is_start = true;
}

void FileSyncEngine::stop()
{
    if(!is_start) return;
    is_start = false;
    std::cout << "FileSyncCore stop" << std::endl;
    //关闭线程
    for (auto& i : file_senders)
    {
        i->stop();
    }
    file_receiver->stop();
    //销毁资源
    file_senders.clear();
    file_receiver.release();
}
