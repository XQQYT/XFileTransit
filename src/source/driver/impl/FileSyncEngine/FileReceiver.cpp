#include "driver/impl/FileSyncEngine/FileReceiver.h"
#include "driver/impl/OuterMsgParser.h"
#include "common/DebugOutputer.h"
#include <iostream>
#include <cstring>
#include <mutex>
#include <algorithm>

#ifdef __linux__
#define MAX_EVENTS 64
#endif

FileReceiver::FileReceiver() : listen_socket(INVALID_SOCKET_VAL), running(false) {}

FileReceiver::~FileReceiver()
{
    closeReceiver();
}

bool FileReceiver::initialize(const std::string &addr, const std::string &p,
                              std::shared_ptr<SecurityInterface> inst)
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
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt)) == SOCKET_ERROR_VAL)
    {
        LOG_ERROR("Failed to set SO_REUSEADDR: " << GET_SOCKET_ERROR);
    }
#else
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        LOG_ERROR("Failed to set SO_REUSEADDR: " << strerror(errno));
    }
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
    tcp_listen_thread = std::make_unique<std::thread>([this, accept_cb, msg_cb]() mutable
                                                      {
#ifdef _WIN32
        std::vector<WSAPOLLFD> poll_fds;
        std::unordered_map<SOCKET, size_t> socket_to_index;
        
        WSAPOLLFD listen_fd;
        listen_fd.fd = listen_socket;
        listen_fd.events = POLLRDNORM;
        listen_fd.revents = 0;
        poll_fds.push_back(listen_fd);
        socket_to_index[listen_socket] = 0;

#else
        int epoll_fd = epoll_create1(0);
        if (epoll_fd == -1) {
            LOG_ERROR("Failed to create epoll: " << strerror(errno));
            running = false;
            return;
        }
        
        struct epoll_event listen_event;
        listen_event.events = EPOLLIN | EPOLLET;
        listen_event.data.fd = listen_socket;
        
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_socket, &listen_event) == -1) {
            LOG_ERROR("Failed to add listen socket to epoll: " << strerror(errno));
            close(epoll_fd);
            running = false;
            return;
        }
        
        std::unordered_map<int, std::shared_ptr<std::thread>> socket_threads;
#endif

        LOG_INFO("FileReceiver started, waiting for connections...");

        while (this->running) {
#ifdef _WIN32
            if (poll_fds.empty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
            
            int result = WSAPoll(poll_fds.data(), poll_fds.size(), 100);
            
            if (result > 0) {
                for (size_t i = 0; i < poll_fds.size(); ++i) {
                    if (poll_fds[i].revents != 0) {
                        if (poll_fds[i].fd == listen_socket) {
                            handleAccept(poll_fds, socket_to_index, accept_cb, msg_cb);
                        } else {
                            if (poll_fds[i].revents & (POLLHUP | POLLERR)) {
                                poll_fds[i].fd = INVALID_SOCKET_VAL;
                            }
                        }
                        poll_fds[i].revents = 0;
                    }
                }
                
                cleanupClosedSockets(poll_fds, socket_to_index);
                
            } else if (result < 0) {
                LOG_ERROR("Error in WSAPoll: " << GET_SOCKET_ERROR);
                if (GET_SOCKET_ERROR != WSAEWOULDBLOCK) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
#else
            struct epoll_event events[MAX_EVENTS];
            int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, 100);
            
            if (nfds > 0) {
                for (int i = 0; i < nfds; ++i) {
                    if (events[i].data.fd == listen_socket) {
                        handleAcceptEpoll(epoll_fd, socket_threads, accept_cb, msg_cb);
                    } else {
                        auto it = socket_threads.find(events[i].data.fd);
                        if (it != socket_threads.end()) {
                            if (it->second->joinable()) {
                                if (events[i].events & EPOLLRDHUP || events[i].events & EPOLLHUP) {
                                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
                                    socket_threads.erase(events[i].data.fd);
                                }
                            } else {
                                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
                                socket_threads.erase(events[i].data.fd);
                            }
                        }
                    }
                }
            } else if (nfds < 0 && errno != EINTR) {
                LOG_ERROR("Error in epoll_wait: " << strerror(errno));
            }
#endif
            
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

#ifdef _WIN32
        for (auto& poll_fd : poll_fds) {
            if (poll_fd.fd != INVALID_SOCKET_VAL) {
                shutdown(poll_fd.fd, SD_BOTH);
                CLOSE_SOCKET(poll_fd.fd);
            }
        }
#else
        for (auto& pair : socket_threads) {
            if (pair.second->joinable()) {
                pair.second->join();
            }
            shutdown(pair.first, SHUT_RDWR);
            CLOSE_SOCKET(pair.first);
        }
        close(epoll_fd);
#endif
        
        if (listen_socket != INVALID_SOCKET_VAL) {
#ifdef _WIN32
            shutdown(listen_socket, SD_BOTH);
#else
            shutdown(listen_socket, SHUT_RDWR);
#endif
            CLOSE_SOCKET(listen_socket);
            listen_socket = INVALID_SOCKET_VAL;
        }
        
        LOG_INFO("FileReceiver listener thread stopped"); });
}

#ifdef _WIN32
void FileReceiver::handleAccept(std::vector<WSAPOLLFD> &poll_fds,
                                std::unordered_map<SOCKET, size_t> &socket_to_index,
                                std::function<void(UnifiedSocket)> &accept_cb,
                                std::function<void(UnifiedSocket, std::unique_ptr<NetworkInterface::UserMsg>)> &msg_cb)
{
    sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    UnifiedSocket accepted_socket = accept(listen_socket,
                                           (sockaddr *)&client_addr, &client_addr_len);

    if (accepted_socket != INVALID_SOCKET_VAL)
    {
        {
            std::lock_guard<std::mutex> lock(sockets_mutex);
            receive_sockets.push_back(accepted_socket);
        }

        LOG_INFO("Accepted connection from "
                 << inet_ntoa(client_addr.sin_addr) << ":"
                 << ntohs(client_addr.sin_port));

        accept_cb(accepted_socket);

        SOCKET_NONBLOCK(accepted_socket);

        int timeout_val = 5000; // 5秒超时
        setsockopt(accepted_socket, SOL_SOCKET, SO_RCVTIMEO,
                   (const char *)&timeout_val, sizeof(timeout_val));

        WSAPOLLFD client_fd;
        client_fd.fd = accepted_socket;
        client_fd.events = POLLRDNORM;
        client_fd.revents = 0;

        poll_fds.push_back(client_fd);
        socket_to_index[accepted_socket] = poll_fds.size() - 1;

        startReceiveThread(accepted_socket, msg_cb);
    }
    else
    {
        int error = GET_SOCKET_ERROR;
        if (error != WSAEWOULDBLOCK)
        {
            LOG_ERROR("Failed to accept connection: " << error);
        }
    }
}

void FileReceiver::handleSocketEvent(SOCKET socket,
                                     std::function<void(UnifiedSocket, std::unique_ptr<NetworkInterface::UserMsg>)> &msg_cb)
{
    WSAPOLLFD fd;
    fd.fd = socket;
    fd.events = POLLRDNORM;
    fd.revents = 0;

    int result = WSAPoll(&fd, 1, 0);
    if (result > 0 && (fd.revents & (POLLHUP | POLLERR)))
    {
        LOG_DEBUG("Socket " << socket << " closed or error detected");
        CLOSE_SOCKET(socket);
    }
}

void FileReceiver::cleanupClosedSockets(std::vector<WSAPOLLFD> &poll_fds,
                                        std::unordered_map<SOCKET, size_t> &socket_to_index)
{
    auto it = poll_fds.begin();
    while (it != poll_fds.end())
    {
        if (it->fd == INVALID_SOCKET_VAL)
        {
            for (auto map_it = socket_to_index.begin(); map_it != socket_to_index.end();)
            {
                if (map_it->second >= std::distance(poll_fds.begin(), it))
                {
                    if (map_it->second == std::distance(poll_fds.begin(), it))
                    {
                        map_it = socket_to_index.erase(map_it);
                    }
                    else
                    {
                        map_it->second--;
                        ++map_it;
                    }
                }
                else
                {
                    ++map_it;
                }
            }
            it = poll_fds.erase(it);
        }
        else
        {
            ++it;
        }
    }
}
#endif

#ifdef __linux__
void FileReceiver::handleAcceptEpoll(int epoll_fd,
                                     std::unordered_map<int, std::shared_ptr<std::thread>> &socket_threads,
                                     std::function<void(UnifiedSocket)> &accept_cb,
                                     std::function<void(UnifiedSocket, std::unique_ptr<NetworkInterface::UserMsg>)> &msg_cb)
{
    while (running)
    {
        sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        UnifiedSocket accepted_socket = accept4(listen_socket,
                                                (sockaddr *)&client_addr, &client_addr_len, SOCK_NONBLOCK);

        if (accepted_socket == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break;
            }
            else
            {
                LOG_ERROR("Failed to accept connection: " << strerror(errno));
                break;
            }
        }

        {
            std::lock_guard<std::mutex> lock(sockets_mutex);
            receive_sockets.push_back(accepted_socket);
        }

        LOG_INFO("Accepted connection from "
                 << inet_ntoa(client_addr.sin_addr) << ":"
                 << ntohs(client_addr.sin_port));

        accept_cb(accepted_socket);

        struct timeval tv;
        tv.tv_sec = 5; // 5秒超时
        tv.tv_usec = 0;
        setsockopt(accepted_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        struct epoll_event event;
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
        event.data.fd = accepted_socket;

        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, accepted_socket, &event) == -1)
        {
            LOG_ERROR("Failed to add socket to epoll: " << strerror(errno));
            CLOSE_SOCKET(accepted_socket);
            continue;
        }

        auto thread = std::make_shared<std::thread>([this, accepted_socket, msg_cb]() mutable
                                                    { receiveThreadFunction(accepted_socket, msg_cb); });

        socket_threads[accepted_socket] = thread;
    }
}
#endif

void FileReceiver::startReceiveThread(UnifiedSocket socket,
                                      std::function<void(UnifiedSocket, std::unique_ptr<NetworkInterface::UserMsg>)> &msg_cb)
{
    auto recv_thread = std::make_shared<std::thread>([this, socket, msg_cb]() mutable
                                                     { receiveThreadFunction(socket, msg_cb); });

    {
        std::lock_guard<std::mutex> lock(threads_mutex);
        receive_threads.push_back(recv_thread);
    }
}

void FileReceiver::receiveThreadFunction(UnifiedSocket socket,
                                         std::function<void(UnifiedSocket, std::unique_ptr<NetworkInterface::UserMsg>)> &msg_cb)
{
    LOG_DEBUG("Receive thread started for socket: " << socket);

    try
    {
        while (running)
        {
#ifdef _WIN32
            WSAPOLLFD fd;
            fd.fd = socket;
            fd.events = POLLRDNORM;
            fd.revents = 0;

            int poll_result = WSAPoll(&fd, 1, 100);
#else
            struct pollfd fd;
            fd.fd = socket;
            fd.events = POLLIN;
            fd.revents = 0;

            int poll_result = poll(&fd, 1, 100);
#endif

            if (!running)
                break;

            if (poll_result > 0)
            {
                if (fd.revents & POLLIN)
                {
                    // 有数据可读，处理数据
                    if (!processSocketData(socket, msg_cb))
                    {
                        break;
                    }
                }
                else if (fd.revents & (POLLHUP | POLLERR | POLLNVAL))
                {
                    // 连接关闭或错误
                    LOG_DEBUG("Socket " << socket << " closed or error in poll");
                    break;
                }
            }
            else if (poll_result < 0)
            {
                int error = GET_SOCKET_ERROR;
                if (error != SOCKET_EWOULDBLOCK)
                {
                    LOG_ERROR("Poll error on socket " << socket << ": " << error);
                    break;
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Receive thread exception on socket " << socket << ": " << e.what());
    }

    LOG_DEBUG("Receive thread ending for socket: " << socket);
    removeSocket(socket);
}

bool FileReceiver::processSocketData(UnifiedSocket socket,
                                     std::function<void(UnifiedSocket, std::unique_ptr<NetworkInterface::UserMsg>)> &msg_cb)
{
    uint8_t peek_buffer[2];
    int peeked = recv(socket, reinterpret_cast<char *>(peek_buffer),
                      sizeof(peek_buffer), MSG_PEEK);

    if (peeked > 0)
    {
        if (peek_buffer[0] == 0xAB && peek_buffer[1] == 0xCD)
        {
            return processProtocolMessage(socket, msg_cb);
        }
        else
        {
            char dump_buffer[4];
            recv(socket, dump_buffer, sizeof(dump_buffer), 0);
            LOG_ERROR("Received invalid data on socket " << socket << ", discarded");
            return true;
        }
    }
    else if (peeked == 0)
    {
        LOG_DEBUG("Peer closed connection on socket: " << socket);
        return false;
    }
    else
    {
        int error = GET_SOCKET_ERROR;
        if (error == SOCKET_EWOULDBLOCK)
        {
            return true;
        }
        LOG_ERROR("Recv error on socket " << socket << ": " << error);
        return false;
    }
}

bool FileReceiver::processProtocolMessage(UnifiedSocket socket,
                                          std::function<void(UnifiedSocket, std::unique_ptr<NetworkInterface::UserMsg>)> &msg_cb)
{
    constexpr int HEADER_SIZE = 8;
    uint8_t buffer[HEADER_SIZE] = {0};
    uint32_t header_received = 0;

    // 接收消息头
    while (header_received < HEADER_SIZE && running)
    {
#ifdef _WIN32
        WSAPOLLFD fd;
        fd.fd = socket;
        fd.events = POLLRDNORM;
        fd.revents = 0;

        if (WSAPoll(&fd, 1, 1000) > 0 && (fd.revents & POLLRDNORM))
        {
#else
        struct pollfd fd;
        fd.fd = socket;
        fd.events = POLLIN;
        fd.revents = 0;

        if (poll(&fd, 1, 1000) > 0 && (fd.revents & POLLIN))
        {
#endif
            int n = recv(socket, reinterpret_cast<char *>(buffer + header_received),
                         HEADER_SIZE - header_received, 0);
            if (n <= 0)
            {
                if (n == 0)
                {
                    LOG_DEBUG("Peer closed connection while receiving header");
                    return false;
                }
                else
                {
                    int err = GET_SOCKET_ERROR;
                    if (err == SOCKET_EWOULDBLOCK)
                    {
                        continue;
                    }
                    LOG_ERROR("Header recv error on socket " << socket << ": " << err);
                    return false;
                }
            }
            header_received += n;
        }
    }

    if (!running)
        return false;
    if (header_received < HEADER_SIZE)
        return true; // 继续等待

    // 解析消息头
    uint32_t payload_length = 0;
    memcpy(&payload_length, buffer + 3, sizeof(payload_length));
    payload_length = ntohl(payload_length);

    uint8_t flag = 0x0;
    memcpy(&flag, buffer + 7, sizeof(flag));

    // 接收消息体
    std::vector<uint8_t> receive_msg(payload_length);
    uint32_t readed_length = 0;

    while (readed_length < payload_length && running)
    {
#ifdef _WIN32
        WSAPOLLFD fd;
        fd.fd = socket;
        fd.events = POLLRDNORM;
        fd.revents = 0;

        if (WSAPoll(&fd, 1, 1000) > 0 && (fd.revents & POLLRDNORM))
        {
#else
        struct pollfd fd;
        fd.fd = socket;
        fd.events = POLLIN;
        fd.revents = 0;

        if (poll(&fd, 1, 1000) > 0 && (fd.revents & POLLIN))
        {
#endif
            int read_byte = recv(socket, reinterpret_cast<char *>(receive_msg.data() + readed_length),
                                 payload_length - readed_length, 0);
            if (read_byte == 0)
            {
                LOG_DEBUG("Peer closed connection while receiving payload");
                return false;
            }
            if (read_byte < 0)
            {
                int error = GET_SOCKET_ERROR;
                if (error == SOCKET_EWOULDBLOCK)
                {
                    continue; // 非阻塞，重试
                }
                LOG_ERROR("Payload recv error on socket " << socket << ": " << error);
                return false;
            }
            readed_length += read_byte;
        }
    }

    if (!running)
        return false;
    if (readed_length < payload_length)
        return true; // 继续等待

    auto parsed = outer_parser->parse(std::move(receive_msg), payload_length, flag);
    memcpy(&parsed->header, buffer, HEADER_SIZE);
    std::vector<uint8_t> result_vec;

    if (flag & static_cast<uint8_t>(NetworkInterface::Flag::IS_ENCRYPT) &&
        security_instance && security_instance->verifyAndDecrypt(parsed->data, security_instance->getTlsInfo().key.get(), parsed->iv, result_vec, parsed->sha256))
    {
        parsed->data.assign(result_vec.begin(), result_vec.end());
        msg_cb(socket, std::move(parsed));
    }
    else
    {
        msg_cb(socket, std::move(parsed));
    }

    return true;
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

    {
        std::lock_guard<std::mutex> lock(sockets_mutex);
        for (auto socket : receive_sockets)
        {
#ifdef _WIN32
            shutdown(socket, SD_BOTH);
#else
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

    // 等待所有接收线程结束
    {
        std::lock_guard<std::mutex> lock(threads_mutex);
        for (auto thread : receive_threads)
        {
            if (thread->joinable())
            {
                // 设置超时，避免死锁
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
                    LOG_ERROR("Thread failed to join in time, detaching");
                    thread->detach();
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