#include "driver/impl/FileSyncEngine/P2P/P2PFileSender.h"
#include "driver/impl/FileSyncEngine/FileMsgBuilder.h"
#include "control/EventBusManager.h"
#include "common/DebugOutputer.h"
#include <iostream>
#include <cerrno>
#include <cstring>

void P2PFileSender::sendMsg(std::vector<uint8_t> &&msg, bool is_binary)
{
    if (msg.empty() || !p2p_inst)
        return;

    NetworkInterface::Flag flag = static_cast<NetworkInterface::Flag>(0);
    if (is_binary)
    {
        flag = static_cast<NetworkInterface::Flag>(static_cast<uint8_t>(flag) |
                                                   static_cast<uint8_t>(NetworkInterface::Flag::IS_BINARY));
    }

    auto ready_to_send_msg = getOuterMsgBuilder().buildMsg(msg, flag);
    if (!ready_to_send_msg)
    {
        LOG_ERROR("Failed to build message");
        return;
    }

    p2p_inst->sendMsg(ready_to_send_msg->data, sender_label);
}

void P2PFileSender::start(std::function<std::optional<std::pair<uint32_t, std::string>>()> get_task_cb)
{
    if (sender_label.empty())
    {
        LOG_ERROR("Failed to get sender label");
        return;
    }
    if (!running)
    {
        running = true;

        send_thread = new std::thread([this, get_task_cb]()
                                      {
            static uint8_t progress_count = 0;
            
            while (running)
            {
                LOG_DEBUG("Checking task");
                // 检查是否有任务
                bool has_task = false;
                {
                    std::unique_lock<std::mutex> lock(mtx);
                    if (check_queue_cb) {
                        has_task = check_queue_cb();
                    }
                }
                
                if (!has_task) {
                    // 没有任务，等待一段时间
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    continue;
                }
                
                auto pending_file = get_task_cb();
                LOG_DEBUG("Got a task");

                if (pending_file.has_value())
                {
                    auto& [id, file_path] = pending_file.value();
                    current_file_id = id;
                    // 设置文件信息
                    file_msg_builder->setFileInfo(id, file_path);
                    
                    // 获取并发送文件流
                    FileMsgBuilderInterface::FileMsgBuilderResult msg;
                    start_time_point = std::chrono::steady_clock::now();
                    bytes_sent = 0;
                    
                    do {
                        if(cancel)
                        {
                            file_msg_builder->cancelSending();
                        }
                        msg = file_msg_builder->getStream();
                        if (msg.data && !msg.data->empty()) {
                            bytes_sent += static_cast<uint32_t>(msg.data->size());
                            sendMsg(std::move(*msg.data), msg.is_binary);
                        }
                        
                        // 每处理40个数据块发送一次进度
                        if (progress_count >= 40)
                        {
                            end_time_point = std::chrono::steady_clock::now();
                            auto elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(
                                end_time_point - start_time_point);
                            
                            uint32_t speed_bps = 0;
                            if (elapsed_us.count() > 0) {
                                uint64_t bps = (static_cast<uint64_t>(bytes_sent) * 1000000ULL) / 
                                              static_cast<uint64_t>(elapsed_us.count());
                                speed_bps = static_cast<uint32_t>(bps);
                                bytes_sent = 0;
                            }
                            
                            start_time_point = std::chrono::steady_clock::now();
                            EventBusManager::instance().publish("/file/upload_progress", 
                                id, msg.progress, speed_bps, false);
                            progress_count = 0;
                        }
                        ++progress_count;
                    } while (msg.data && !msg.data->empty());
                    if(!cancel)
                    {
                        // 发送完成事件
                        EventBusManager::instance().publish("/file/upload_progress", 
                            id, static_cast<uint8_t>(100), static_cast<uint32_t>(0), true);
                    }
                    progress_count = 0;
                    current_file_id = std::nullopt;
                    cancel = false;
                }
            }
            
            LOG_INFO("P2PFileSender thread exited"); });
    }
}

void P2PFileSender::stop()
{
    running = false;
    if (send_thread)
    {
        cv.notify_all();
    }
}

P2PFileSender::~P2PFileSender()
{
    stop();
    LOG_INFO("P2PFileSender destroyed");
}