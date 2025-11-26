#ifndef _PARSER_H
#define _PARSER_H

#include <memory>
#include "driver/interface/NetworkInterface.h"

class Parser
{
public:
    virtual void parse(std::unique_ptr<NetworkInterface::UserMsg> data) = 0;
};

#endif