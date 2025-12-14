#ifndef _FILESYNCENGINE_H
#define _FILESYNCENGINE_H

#include "driver/interface/SecurityInterface.h"
#include "driver/interface/NetworkInterface.h"
#include "driver/interface/FileSyncEngine/FileSenderInterface.h"
#include "driver/interface/FileSyncEngine/FileReceiverInterface.h"
#include "driver/interface/FileSyncEngine/FileParserInterface.h"
#include <queue>
#include <condition_variable>
#include <mutex>
#include <utility>

static const uint8_t sender_num = 4;

class FileSyncEngine
{
public:
    FileSyncEngine();
    void start(std::string address, std::string recv_port, std::shared_ptr<SecurityInterface> instance);
    void stop();
    void onHaveFileToSend(uint32_t id, std::string path);
    std::optional<std::pair<uint32_t, std::string>> getPendingFile();
    void haveFileConnection(UnifiedSocket socket);
    void haveFileMsg(UnifiedSocket socket, std::unique_ptr<NetworkInterface::UserMsg> msg);
    void setConcurrentTask(uint8_t num);
    ~FileSyncEngine();

private:
    std::vector<std::shared_ptr<FileSenderInterface>> file_senders;
    std::unique_ptr<FileReceiverInterface> file_receiver;
    std::unordered_map<UnifiedSocket, std::unique_ptr<FileParserInterface>> file_parser_map;
    std::shared_ptr<std::condition_variable> cv;
    std::mutex mtx;

    std::string address;
    std::string recv_port;
    std::shared_ptr<SecurityInterface> instance;

private:
    std::queue<std::pair<uint32_t, std::string>>
        pending_send_files;
    bool is_start{false};
};

#endif