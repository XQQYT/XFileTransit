#ifndef FILERECEIVER_H
#define FILERECEIVER_H

#include "driver/interface/FileSyncEngine/FileReceiverInterface.h"

class FileReceiver : public FileReceiverInterface
{
public:
    using FileReceiverInterface::FileReceiverInterface;
    bool initialize() override;
    void start(std::function<void(uint32_t id, float progress)> progress_cb) override;
    void stop() override;
};

#endif