#ifndef _FILESYNCENGINE_H
#define _FILESYNCENGINE_H

#include "driver/interface/SecurityInterface.h"

class FileSyncEngine
{
public:
    void start(std::string address, std::string send_port, std::string recv_port, SecurityInterface::TlsInfo info);
};

#endif