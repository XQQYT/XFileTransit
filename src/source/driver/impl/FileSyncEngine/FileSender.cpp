#include "driver/impl/FileSyncEngine/FileSender.h"
#include "driver/impl/FileSyncEngine/FileMsgBuilder.h"
#include "control/EventBusManager.h"
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>


bool FileSender::initialize()
{
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
    {
        std::cerr << "WSAStartup failed\n";
        return false;
    }
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    client_tcp_addr.sin_family = AF_INET;
    client_tcp_addr.sin_port = htons(std::stoi(port));
    inet_pton(AF_INET, address.c_str(), &client_tcp_addr.sin_addr);
    connect(client_socket, (sockaddr*)&client_tcp_addr, sizeof(client_tcp_addr));
    file_msg_builder = std::make_unique<FileMsgBuilder>();
    getOuterMsgBuilder().setSecurityInstance(security_instance);
    return true;
}
void FileSender::sendMsg(std::vector<uint8_t>&& msg, bool is_binary)
{
    if (msg.size() <= 0)
        return;
    NetworkInterface::Flag flag = NetworkInterface::Flag::IS_ENCRYPT;
    flag = is_binary ? flag | NetworkInterface::Flag::IS_BINARY : flag;
    std::unique_ptr<NetworkInterface::UserMsg> ready_to_send_msg = std::move(getOuterMsgBuilder().buildMsg(msg, flag));

    size_t final_msg_length = ready_to_send_msg->data.size();
    size_t sended_length = 0;

    while (sended_length < final_msg_length)
    {
        int ret = send(client_socket, reinterpret_cast<const char*>(ready_to_send_msg->data.data() + sended_length),
            final_msg_length - sended_length, 0);
        if (ret <= 0)
        {
            if (errno == EINTR)
                continue;
            perror("write failed");
            break;
        }
        sended_length += ret;
    }
}
void FileSender::start(std::function<std::optional<std::pair<uint32_t, std::string>>()> get_task_cb)
{
    running = true;
    send_thread = new std::thread([=]() {
        static uint8_t progress_count = 0;
        while (running)
        {
            std::unique_lock<std::mutex> lock(mtx);

            cv->wait(lock, [this]() {
                bool has_task = check_queue_cb();
                bool should_stop = !running;
                return has_task || should_stop;
                });
            auto pending_file = get_task_cb();
            if (pending_file.has_value())
            {
                auto& [id, file_path] = pending_file.value();
                file_msg_builder->setFileInfo(id, file_path);
                FileMsgBuilderInterface::FileMsgBuilderResult msg;
                start_time_point = std::chrono::steady_clock::now();
                do {
                    msg = file_msg_builder->getStream();
                    if (msg.data) {
                        bytes_sent += msg.data->size();
                        sendMsg(std::move(*msg.data), msg.is_binary);
                    }
                    if (progress_count >= 40)
                    {
                        end_time_point = std::chrono::steady_clock::now();
                        auto elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(end_time_point - start_time_point);
                        uint32_t speed_bps = 0;
                        if (elapsed_us.count() > 0) {
                            uint64_t bps = (static_cast<uint64_t>(bytes_sent) * 1000000ULL) / elapsed_us.count();
                            speed_bps = static_cast<uint32_t>(bps);
                            bytes_sent = 0;
                        }
                        start_time_point = std::chrono::steady_clock::now();
                        EventBusManager::instance().publish("/file/upload_progress", id, msg.progress, speed_bps, false);
                        progress_count = 0;
                    }
                    ++progress_count;
                } while (msg.data);
                EventBusManager::instance().publish("/file/upload_progress", id, static_cast <uint8_t>(100), static_cast<uint32_t>(0), true);
                progress_count = 0;
            }
            else
            {
                continue;
            }
        }
        });
}

void FileSender::stop()
{
    running = false;
}

FileSender::~FileSender()
{
    if (send_thread->joinable())
    {
        send_thread->join();
    }
    delete send_thread;
    closesocket(client_socket);
}