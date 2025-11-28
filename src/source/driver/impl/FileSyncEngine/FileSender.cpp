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
    return true;
}

void FileSender::start(std::function<std::pair<uint32_t, std::string>()> get_task_cb)
{
    running = true;
    send_thread = new std::thread([=]() {
        while (running)
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv->wait(lock);
            auto pending_file = get_task_cb();
            std::cout << std::this_thread::get_id() << " sended " << pending_file.first << " path " << pending_file.second << std::endl;
        }
        });
}

void FileSender::stop()
{

}