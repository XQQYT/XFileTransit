#ifndef OUTERMSGPARSERINTERFACE_H
#define OUTERMSGPARSERINTERFACE_H

#include <functional>
#include <vector>
#include <memory>

#include "driver/interface/OuterMsgBuilderInterface.h"

class OuterMsgParserInterface
{
public:
    struct ParsedMsg {
        std::vector<uint8_t> iv;
        std::vector<uint8_t> data;
        std::vector<uint8_t> sha256;
        OuterMsgBuilderInterface::Header header;
    };
    virtual std::unique_ptr<ParsedMsg> parse(std::vector<uint8_t>&& msg, const uint32_t length, const uint8_t flag) = 0;
};


#endif