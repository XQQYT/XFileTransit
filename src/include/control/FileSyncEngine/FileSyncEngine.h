#ifndef _FILESYNCENGINE_H
#define _FILESYNCENGINE_H

#include "driver/interface/SecurityInterface.h"
#include "driver/interface/OuterMsgBuilderInterface.h"
#include "driver/interface/OuterMsgParserInterface.h"
#include "driver/interface/FileSyncEngine/FileSenderInterface.h"
#include "driver/interface/FileSyncEngine/FileReceiverInterface.h"
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
private:
    std::vector<FileSenderInterface> file_senders;
    std::unique_ptr<FileReceiverInterface> file_receiver;
    std::shared_ptr<std::condition_variable> cv;
    std::mutex mtx;
private:
    std::shared_ptr<OuterMsgBuilderInterface> outer_msg_builder;
    std::shared_ptr<OuterMsgParserInterface> outer_msg_parser;
    std::queue<std:pair<uint32_t,std::string>> pending_send_files;
};

#endif