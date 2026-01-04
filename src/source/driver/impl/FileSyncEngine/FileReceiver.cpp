#include "driver/impl/FileSyncEngine/FileReceiver.h"
#include "driver/impl/OuterMsgParser.h"
#include "common/DebugOutputer.h"
#include <iostream>
#include <cstring>
#include <mutex>

bool FileReceiver::initialize(const std::string &addr, const std::string &p, std::shared_ptr<SecurityInterface> inst)
{
    address = addr;
    port = p;
    security_instance = inst;
    listen_socket = createListenSocket(address, port);
    outer_parser = std::make_unique<OuterMsgParser>();
    return (listen_socket != INVALID_SOCKET_VAL);
}

UnifiedSocket FileReceiver::createListenSocket(const std::string &address, const std::string &port)
{
    // 创建socket
    UnifiedSocket sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET_VAL)
    {
        LOG_ERROR("Failed to create socket: " << GET_SOCKET_ERROR);
        return INVALID_SOCKET_VAL;
    }

    // 设置SO_REUSEADDR选项，避免端口占用
    int opt = 1;
#ifdef _WIN32
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));
#else
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(std::stoi(port));

    // 解析地址
    if (address.empty() || address == "0.0.0.0")
    {
        addr.sin_addr.s_addr = INADDR_ANY;
    }
    else
    {
        if (inet_pton(AF_INET, address.c_str(), &addr.sin_addr) <= 0)
        {
            LOG_ERROR("Invalid address: " << address);
            CLOSE_SOCKET(sock);
            return INVALID_SOCKET_VAL;
        }
    }

    // 绑定socket
    if (bind(sock, (sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR_VAL)
    {
        LOG_ERROR("Bind failed: " << GET_SOCKET_ERROR);
        CLOSE_SOCKET(sock);
        return INVALID_SOCKET_VAL;
    }

    // 监听socket
    if (listen(sock, 5) == SOCKET_ERROR_VAL)
    {
        LOG_ERROR("Listen failed: " << GET_SOCKET_ERROR);
        CLOSE_SOCKET(sock);
        return INVALID_SOCKET_VAL;
    }

    LOG_INFO("Listen socket created successfully on " << address << ":" << port);
    return sock;
}

void FileReceiver::start(std::function<void(UnifiedSocket)> accept_cb,
                         std::function<void(UnifiedSocket socket, std::unique_ptr<NetworkInterface::UserMsg>)> msg_cb)
{
    running = true;
    tcp_listen_thread = std::make_unique<std::thread>([this, accept_cb = std::move(accept_cb), msg_cb = std::move(msg_cb)]()
                                                      {
#ifdef _WIN32
            // Windows使用WSAPoll
            WSAPOLLFD fds[1];
            fds[0].fd = listen_socket;
            fds[0].events = POLLRDNORM;
#else
            // Linux使用poll
            pollfd fds[1];
            fds[0].fd = listen_socket;
            fds[0].events = POLLIN;
#endif

            while (this->running)
            {
#ifdef _WIN32
                int result = WSAPoll(fds, 1, 100);
#else
                int result = poll(fds, 1, 100);
#endif

                if (result > 0) {
#ifdef _WIN32
                    bool ready = (fds[0].revents & POLLRDNORM) != 0;
#else
                    bool ready = (fds[0].revents & POLLIN) != 0;
#endif
                    if (ready)
                    {
                        sockaddr_in client_addr;
                        socklen_t client_addr_len = sizeof(client_addr);
                        
                        UnifiedSocket accepted_socket = accept(listen_socket, 
                            (sockaddr*)&client_addr, &client_addr_len);
                            
                        if (accepted_socket != INVALID_SOCKET_VAL)
                        {
                            {
                                std::lock_guard<std::mutex> lock(sockets_mutex);
                                receive_sockets.push_back(accepted_socket);
                            }
                            
                            LOG_INFO("Accepted connection from " 
                                      << inet_ntoa(client_addr.sin_addr) << ":" 
                                      << ntohs(client_addr.sin_port));
                            
                            // 调用accept回调
                            accept_cb(accepted_socket);
                            
                            // 创建接收线程
                            std::shared_ptr<std::thread> recv_thread = std::make_shared<std::thread>([this, accepted_socket, msg_cb]() {
                                bool thread_running = true;
                                
                                // 为每个连接设置socket选项
                                int timeout_val = 5000; // 5秒超时
#ifdef _WIN32
                                setsockopt(accepted_socket, SOL_SOCKET, SO_RCVTIMEO, 
                                          (const char*)&timeout_val, sizeof(timeout_val));
#else
                                struct timeval tv;
                                tv.tv_sec = timeout_val / 1000;
                                tv.tv_usec = (timeout_val % 1000) * 1000;
                                setsockopt(accepted_socket, SOL_SOCKET, SO_RCVTIMEO, 
                                          &tv, sizeof(tv));
#endif

                                outer_parser->delegateRecv(accepted_socket,
                                    [accepted_socket, msg_cb](std::unique_ptr<NetworkInterface::UserMsg> parsed_msg) {
                                        msg_cb(accepted_socket, std::move(parsed_msg));
                                    }, 
                                    [this, accepted_socket]() {
                                        // 连接关闭回调
                                        LOG_INFO("Connection closed for socket: " << accepted_socket);
                                        this->removeSocket(accepted_socket);
                                    },
                                    [](const NetworkInterface::RecvError error) {
                                        // 接收错误回调
                                        LOG_ERROR("Receive error: " << static_cast<int>(error));
                                    },
                                    security_instance, thread_running);
                                    
                            });
                            
                            {
                                std::lock_guard<std::mutex> lock(threads_mutex);
                                receive_threads.push_back(recv_thread);
                            }
                        }
                        else
                        {
                            LOG_ERROR("Failed to accept connection: " << GET_SOCKET_ERROR);
                        }
                    }
                }
                else if (result < 0)
                {
                    LOG_ERROR("Error in poll: " << GET_SOCKET_ERROR);
                    break;
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            CLOSE_SOCKET(listen_socket); });
}

void FileReceiver::removeSocket(UnifiedSocket socket)
{
    std::lock_guard<std::mutex> lock(sockets_mutex);
    auto it = std::find(receive_sockets.begin(), receive_sockets.end(), socket);
    if (it != receive_sockets.end())
    {
        CLOSE_SOCKET(socket);
        receive_sockets.erase(it);
        LOG_INFO("Socket " << socket << " removed");
    }
}

void FileReceiver::closeReceiver()
{
    running = false;

    // 先关闭所有socket，这会中断阻塞的select/poll调用
    {
        std::lock_guard<std::mutex> lock(sockets_mutex);
        for (auto socket : receive_sockets)
        {
            // 设置socket为异步关闭模式
#ifdef _WIN32
            // Windows上使用shutdown强制关闭
            shutdown(socket, SD_BOTH);
#else
            // Linux上使用shutdown中断阻塞调用
            shutdown(socket, SHUT_RDWR);
#endif
            CLOSE_SOCKET(socket);
        }
        receive_sockets.clear();
    }

    // 关闭监听socket
    if (listen_socket != INVALID_SOCKET_VAL)
    {
#ifdef _WIN32
        shutdown(listen_socket, SD_BOTH);
#else
        shutdown(listen_socket, SHUT_RDWR);
#endif
        CLOSE_SOCKET(listen_socket);
        listen_socket = INVALID_SOCKET_VAL;
    }

    // 等待监听线程结束
    if (tcp_listen_thread)
    {
        if (tcp_listen_thread->joinable())
        {
            tcp_listen_thread->join();
        }
        tcp_listen_thread.reset();
        tcp_listen_thread = nullptr;
    }

    // 等待所有接收线程结束，设置超时避免无限等待
    {
        std::lock_guard<std::mutex> lock(threads_mutex);
        for (auto thread : receive_threads)
        {
            if (thread->joinable())
            {
                // 设置超时，避免死锁
                if (thread->joinable())
                {
                    // 尝试等待2秒
                    auto start_time = std::chrono::steady_clock::now();
                    bool thread_joined = false;

                    while (std::chrono::steady_clock::now() - start_time < std::chrono::seconds(2))
                    {
                        if (thread->joinable())
                        {
                            thread->join();
                            thread_joined = true;
                            break;
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }

                    if (!thread_joined)
                    {
                        LOG_ERROR("Warning: Thread failed to join in time, detaching");
                        thread->detach();
                    }
                }
            }
            thread.reset();
        }
        receive_threads.clear();
    }

    LOG_INFO("FileReceiver closed successfully");
}

void FileReceiver::stop()
{
    closeReceiver();
}