#include "driver/impl/FileSyncEngine/FileReceiver.h"
#include "driver/impl/OuterMsgParser.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

bool FileReceiver::initialize()
{
    listen_socket = createListenSocket(address, port);
    outer_parser = std::make_unique<OuterMsgParser>();
    return true;
}

SOCKET FileReceiver::createListenSocket(const std::string& address, const std::string& port) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) return INVALID_SOCKET;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(std::stoi(port));
    inet_pton(AF_INET, address.c_str(), &addr.sin_addr);

    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        closesocket(sock);
        return INVALID_SOCKET;
    }

    if (listen(sock, 5) == SOCKET_ERROR) {
        closesocket(sock);
        return INVALID_SOCKET;
    }

    return sock;
}

void FileReceiver::start(std::function<void(SOCKET)> accept_cb,
    std::function<void(SOCKET socket, std::unique_ptr<NetworkInterface::UserMsg>)> msg_cb)
{
    running = true;
    tcp_listen_thread = new std::thread([this, accept_cb = std::move(accept_cb), msg_cb = std::move(msg_cb)]()
        {
            WSAPOLLFD fds[1];
            fds[0].fd = listen_socket;
            fds[0].events = POLLRDNORM;

            while (this->running)
            {
                int result = WSAPoll(fds, 1, 100);

                if (result > 0 && (fds[0].revents & POLLRDNORM))
                {
                    int accept_addr_len = sizeof(accept_addr);
                    SOCKET accepted_socket = accept(listen_socket, (sockaddr*)&accept_addr, &accept_addr_len);
                    if (accepted_socket != INVALID_SOCKET)
                    {
                        receive_sockets.push_back(accepted_socket);
                        std::cout << "accept a socket" << std::endl;
                        accept_cb(accepted_socket);
                        std::thread* t = new std::thread([=]() {
                            while (running)
                            {
                                outer_parser->delegateRecv(accepted_socket,
                                    [accepted_socket, msg_cb](std::unique_ptr<NetworkInterface::UserMsg> parsed_msg) {
                                        msg_cb(accepted_socket, std::move(parsed_msg));
                                    }, nullptr, nullptr, security_instance);
                            }
                            });
                    }
                    else
                    {
                        std::cerr << "fail to accept" << WSAGetLastError() << std::endl;
                    }
                }
                else if (result < 0)
                {
                    std::cerr << "error in WSAPoll" << WSAGetLastError() << std::endl;
                }
            }

            closesocket(listen_socket); });
}

void FileReceiver::closeReceiver()
{
    running = false;
    for (auto it = receive_threads.begin(); it < receive_threads.end(); it++)
    {
        if ((*it)->joinable())
        {
            (*it)->join();
        }
        receive_threads.erase(it);
    }
    if (tcp_listen_thread->joinable())
    {
        tcp_listen_thread->join();
    }
    for (auto& i : receive_sockets)
    {
        closesocket(i);
    }
    closesocket(listen_socket);
}

void FileReceiver::stop()
{
    closeReceiver();
}