#ifndef OUTERMSGPARSER_H
#define OUTERMSGPARSER_H

#include "driver/interface/OuterMsgParserInterface.h"

class OuterMsgParser : public OuterMsgParserInterface
{
public:
    std::unique_ptr<OuterMsgParserInterface::ParsedMsg> parse(std::vector<uint8_t>&& msg, const uint32_t length, const uint8_t flag) override;
};

#endif