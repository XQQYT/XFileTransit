#include "control/FileSyncEngine/FileSyncEngine.h"
#include "driver/impl/OuterMsgBuilder.h"
#include "driver/impl/OuterMsgParser.h"
#include "driver/impl/FileSyncEngine/FileSender.h"
#include "driver/impl/FileSyncEngine/FileReceiver.h"
#include "driver/impl/FileSyncEngine/FileParser.h"
#include "control/EventBusManager.h"
#include <iostream>

FileSyncEngine::FileSyncEngine()
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

std::optional<std::pair<uint32_t, std::string>> FileSyncEngine::getPendingFile()
{
    std::lock_guard<std::mutex> lock(mtx);
    if (pending_send_files.empty()) {
        return std::nullopt;
    }
    auto file = pending_send_files.front();
    pending_send_files.pop();
    return file;
}

void FileSyncEngine::haveFileConnection(SOCKET socket)
{
    if (file_parser_map.find(socket) == file_parser_map.end())
    {
        file_parser_map[socket] = std::make_unique<FileParser>();
    }
}

void FileSyncEngine::haveFileMsg(SOCKET socket, std::unique_ptr<NetworkInterface::UserMsg> msg)
{
    file_parser_map[socket]->parse(std::move(msg));
}

void FileSyncEngine::start(std::string address, std::string recv_port,
    std::shared_ptr<SecurityInterface> instance)
{
    std::cout << "FileSyncCore start" << std::endl;

    //初始化receiver
    file_receiver = std::make_unique<FileReceiver>("0.0.0.0", recv_port, instance);
    cv = std::make_shared<std::condition_variable>();

    if (file_receiver->initialize())
    {
        file_receiver->start(std::bind(&FileSyncEngine::haveFileConnection, this, std::placeholders::_1),
            std::bind(&FileSyncEngine::haveFileMsg, this, std::placeholders::_1, std::placeholders::_2));
    }

    //初始化sender
    std::vector<std::shared_ptr<FileSender>> initialized_senders;
    for (int i = 0; i < sender_num; ++i)
    {
        auto sender = std::make_shared<FileSender>(address, recv_port, instance);
        if (sender->initialize())
        {
            sender->setCondition(this->cv);
            sender->setCheckQueue([this]()->bool {
                std::lock_guard<std::mutex> lock(mtx);
                bool is_empty = pending_send_files.empty();
                return !is_empty;
                });
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
    if (!is_start) return;
    is_start = false;
    std::cout << "FileSyncCore stop" << std::endl;
    //关闭线程
    for (auto& i : file_senders)
    {
        i->stop();
    }
    cv->notify_all();
    file_receiver->stop();
    //销毁资源
    file_senders.clear();
    file_receiver.release();
    cv.reset();
}

FileSyncEngine::~FileSyncEngine()
{
    stop();
}