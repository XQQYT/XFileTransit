#include "driver/impl/FileSyncEngine/FileSender.h"
#include <iostream>

bool FileSender::initialize()
{
    client_tcp_addr.sin_family = AF_INET;
    client_tcp_addr.sin_port = htons(std::stoi(port));
    inet_pton(AF_INET, address.c_str(), &client_tcp_addr.sin_addr);
    connect(client_socket, nullptr, sizeof(client_tcp_addr));
}

void FileSender::start(std::function<std::pair<uint32_t,std::string>()> get_task_cb)
{
    running = true;
    while(running)
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv->wait(lock);
        auto pending_file = get_task_cb();
        std::cout<<"发送 "<< pending_file.first <<" path "<<pending_file.second<<std::endl;
    }
}

void FileSender::stop()
{

}