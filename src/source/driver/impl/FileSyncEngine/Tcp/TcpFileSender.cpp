#include "driver/impl/FileSyncEngine/Tcp/TcpFileSender.h"
#include "driver/impl/FileSyncEngine/FileMsgBuilder.h"
#include "control/EventBusManager.h"
#include "driver/interface/PlatformSocket.h"
#include "common/DebugOutputer.h"
#include <iostream>
#include <cerrno>
#include <cstring>

bool TcpFileSender::initialize(const std::string &addr, const std::string &p, std::shared_ptr<SecurityInterface> inst)
{
    address = addr;
    port = p;
    security_instance = inst;
#ifdef _WIN32
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
    {
        LOG_ERROR("WSAStartup failed: " << GET_SOCKET_ERROR);
        return false;
    }
#endif

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET_VAL)
    {
        LOG_ERROR("Failed to create socket: " << GET_SOCKET_ERROR);
        return false;
    }

    client_tcp_addr.sin_family = AF_INET;
    client_tcp_addr.sin_port = htons(static_cast<uint16_t>(std::stoi(port)));

    if (inet_pton(AF_INET, address.c_str(), &client_tcp_addr.sin_addr) <= 0)
    {
        LOG_ERROR("Invalid address: " << address);
        CLOSE_SOCKET(client_socket);
        client_socket = INVALID_SOCKET_VAL;
        return false;
    }

    if (connect(client_socket, (sockaddr *)&client_tcp_addr, sizeof(client_tcp_addr)) == SOCKET_ERROR_VAL)
    {
        LOG_ERROR("Connect failed: " << GET_SOCKET_ERROR);
        CLOSE_SOCKET(client_socket);
        client_socket = INVALID_SOCKET_VAL;
        return false;
    }

    file_msg_builder = std::make_unique<FileMsgBuilder>();
    if (security_instance)
    {
        getOuterMsgBuilder().setSecurityInstance(security_instance);
    }

    LOG_INFO("TcpFileSender initialized successfully");
    return true;
}

void TcpFileSender::sendMsg(std::vector<uint8_t> &&msg, bool is_binary)
{
    if (msg.empty() || client_socket == INVALID_SOCKET_VAL)
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

    size_t final_msg_length = ready_to_send_msg->data.size();
    size_t sended_length = 0;

    while (sended_length < final_msg_length && running)
    {
        int ret = send(client_socket,
                       reinterpret_cast<const char *>(ready_to_send_msg->data.data() + sended_length),
                       static_cast<int>(final_msg_length - sended_length), 0);
        if (ret <= 0)
        {
            int err = GET_SOCKET_ERROR;
            if (err == SOCKET_EINTR)
            {
                continue; // 被信号中断，重试
            }
            LOG_ERROR("Send failed, error: " << err);
            break;
        }
        sended_length += ret;
    }
}

void TcpFileSender::start(std::function<std::optional<std::pair<uint32_t, std::string>>()> get_task_cb)
{
    if (!running)
    {
        running = true;

        send_thread = new std::thread([this, get_task_cb]()
                                      {
            static uint8_t progress_count = 0;
            
            while (running)
            {
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
            
            LOG_INFO("TcpFileSender thread exited"); });
    }
}

void TcpFileSender::stop()
{
    running = false;
    if (send_thread)
    {
        cv.notify_all();
    }
}

TcpFileSender::~TcpFileSender()
{
    stop();

    if (send_thread)
    {
        if (send_thread->joinable())
        {
            send_thread->join();
        }
        delete send_thread;
        send_thread = nullptr;
    }

    if (client_socket != INVALID_SOCKET_VAL)
    {
        CLOSE_SOCKET(client_socket);
        client_socket = INVALID_SOCKET_VAL;
    }

#ifdef _WIN32
    WSACleanup();
#endif

    LOG_INFO("TcpFileSender destroyed");
}