#include "driver/impl/FileSyncEngine/FileParser.h"
#include <string>
#include <iostream>

void FileParser::parse(std::unique_ptr<NetworkInterface::UserMsg> msg)
{
    std::cout << "File Msg  " << std::string(msg->data.data(), msg->data.data() + msg->data.size()) << std::endl;
}