#ifndef FILEPARSERINTERFACE_H
#define FILEPARSERINTERFACE_H

#include <vector>
#include <stdint.h>
#include "driver/interface/Network/NetworkInterface.h"

class FileParserInterface
{
public:
    virtual void parse(std::unique_ptr<NetworkInterface::UserMsg> msg) = 0;
    virtual ~FileParserInterface() = default;
};

#endif