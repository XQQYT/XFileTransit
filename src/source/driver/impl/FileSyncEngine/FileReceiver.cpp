#include "driver/impl/FileSyncEngine/FileReceiver.h"

bool FileReceiver::initialize()
{
    return true;
}

void FileReceiver::start(std::function<void(uint32_t id, float progress)> progress_cb)
{

}

void FileReceiver::stop()
{

}