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
    EventBusManager::instance().subscribe("/file/cancel_file_send", std::bind(
                                                                        &FileSyncEngine::onCancelSendFile,
                                                                        this,
                                                                        std::placeholders::_1));
    // 被动修改方
    EventBusManager::instance().subscribe("/settings/have_concurrent_changed", std::bind(
                                                                                   &FileSyncEngine::setConcurrentTask,
                                                                                   this,
                                                                                   std::placeholders::_1));
    // 修改方
    EventBusManager::instance().subscribe("/settings/send_concurrent_changed", std::bind(
                                                                                   &FileSyncEngine::setConcurrentTask,
                                                                                   this,
                                                                                   std::placeholders::_1));
}

void FileSyncEngine::onHaveFileToSend(uint32_t id, std::string path)
{
    std::lock_guard<std::mutex> lock(mtx);
    pending_send_files.push_back({id, std::move(path)});
    cv->notify_one();
}

void FileSyncEngine::onCancelSendFile(uint32_t id)
{
    std::lock_guard<std::mutex> lock(mtx);

    auto new_end = std::remove_if(pending_send_files.begin(),
                                  pending_send_files.end(),
                                  [id](const std::pair<uint32_t, std::string> &element)
                                  {
                                      return id == element.first;
                                  });
    pending_send_files.erase(new_end, pending_send_files.end());
}

std::optional<std::pair<uint32_t, std::string>> FileSyncEngine::getPendingFile()
{
    std::lock_guard<std::mutex> lock(mtx);
    if (pending_send_files.empty())
    {
        return std::nullopt;
    }
    auto file = pending_send_files.front();
    pending_send_files.pop_front();
    return file;
}

void FileSyncEngine::haveFileConnection(UnifiedSocket socket)
{
    if (file_parser_map.find(socket) == file_parser_map.end())
    {
        file_parser_map[socket] = std::make_unique<FileParser>();
    }
}

void FileSyncEngine::haveFileMsg(UnifiedSocket socket, std::unique_ptr<NetworkInterface::UserMsg> msg)
{
    file_parser_map[socket]->parse(std::move(msg));
}

void FileSyncEngine::start(std::string address, std::string recv_port,
                           std::shared_ptr<SecurityInterface> instance)
{
    this->address = address;
    this->recv_port = recv_port;
    this->instance = instance;

    LOG_INFO("FileSyncCore start");

    // 初始化receiver
    file_receiver = std::make_unique<FileReceiver>("0.0.0.0", this->recv_port, this->instance);
    cv = std::make_shared<std::condition_variable>();

    if (file_receiver->initialize())
    {
        file_receiver->start(std::bind(&FileSyncEngine::haveFileConnection, this, std::placeholders::_1),
                             std::bind(&FileSyncEngine::haveFileMsg, this, std::placeholders::_1, std::placeholders::_2));
    }

    // 初始化sender
    std::vector<std::shared_ptr<FileSender>> initialized_senders;
    for (int i = 0; i < sender_num; ++i)
    {
        auto sender = std::make_shared<FileSender>(this->address, this->recv_port, this->instance);
        if (sender->initialize())
        {
            sender->setCondition(this->cv);
            sender->setCheckQueue([this]() -> bool
                                  {
                std::lock_guard<std::mutex> lock(mtx);
                bool is_empty = pending_send_files.empty();
                return !is_empty; });
            initialized_senders.push_back(sender);
        }
    }

    for (auto &sender : initialized_senders)
    {
        sender->start(std::bind(&FileSyncEngine::getPendingFile, this));
        file_senders.push_back(sender);
    }
    is_start = true;
}

void FileSyncEngine::setConcurrentTask(uint8_t num)
{
    LOG_DEBUG("new concurrent nums is " << static_cast<int>(num));
    int16_t need_to_close = num - file_senders.size();
    // 增加并行数
    if (need_to_close >= 0)
    {
        std::vector<std::shared_ptr<FileSender>> initialized_senders;
        for (uint8_t i = 0; i < need_to_close; ++i)
        {
            LOG_DEBUG("add one sender");
            auto sender = std::make_shared<FileSender>(this->address, this->recv_port, this->instance);
            if (sender->initialize())
            {
                sender->setCondition(this->cv);
                sender->setCheckQueue([this]() -> bool
                                      {
                    std::lock_guard<std::mutex> lock(mtx);
                    bool is_empty = pending_send_files.empty();
                    return !is_empty; });
                initialized_senders.push_back(sender);
            }
        }
        for (auto &sender : initialized_senders)
        {
            sender->start(std::bind(&FileSyncEngine::getPendingFile, this));
            file_senders.push_back(sender);
        }
    }
    else // 减少并行数
    {
        need_to_close = std::abs(need_to_close);
        for (uint8_t i = 0; i < need_to_close; ++i)
        {
            LOG_DEBUG("remove one sender");
            auto sender = file_senders.back();
            sender->stop();
            file_senders.pop_back();
        }
    }
}

void FileSyncEngine::stop()
{
    if (!is_start)
        return;
    is_start = false;
    LOG_INFO("FileSyncCore stop");
    // 关闭线程
    for (auto &i : file_senders)
    {
        i->stop();
    }
    cv->notify_all();
    file_receiver->stop();
    // 销毁资源
    file_senders.clear();
    file_receiver.release();
    cv.reset();
}

FileSyncEngine::~FileSyncEngine()
{
    stop();
}