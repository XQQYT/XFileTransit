#ifndef FILERECEIVER_H
#define FILERECEIVER_H

#include "driver/interface/FileSyncEngine/FileReceiverInterface.h"

class FileReceiver : public FileReceiverInterface
{
public:
    using FileReceiverInterface::FileReceiverInterface;
    bool initialize() override;
    void start(std::function<std::string()> receiveded_cb) override;
    void stop() override;
};

#endif