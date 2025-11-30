#include "driver/impl/FileSyncEngine/FileSender.h"
#include "driver/impl/FileSyncEngine/FileMsgBuilder.h"
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
void FileSender::sendMsg(std::vector<uint8_t>&& msg)
{
    std::unique_ptr<NetworkInterface::UserMsg> ready_to_send_msg = std::move(getOuterMsgBuilder().buildMsg(msg));

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
        std::cout << sended_length << " / " << final_msg_length << std::endl;
    }
}
void FileSender::start(std::function<std::optional<std::pair<uint32_t, std::string>>()> get_task_cb)
{
    running = true;
    send_thread = new std::thread([=]() {
        while (running)
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv->wait(lock);
            auto pending_file = get_task_cb();
            if (pending_file.has_value())
            {
                auto& [id, file_path] = pending_file.value();
                file_msg_builder->setFileInfo(id, file_path);
                while (auto msg = file_msg_builder->getStream())
                {
                    sendMsg(std::move(*msg));
                }
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