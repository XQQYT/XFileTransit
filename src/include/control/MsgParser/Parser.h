#ifndef _PARSER_H
#define _PARSER_H

#include <memory>
#include "driver/interface/OuterMsgParserInterface.h"

class Parser
{
public:
    virtual void parse(std::unique_ptr<OuterMsgParserInterface::ParsedMsg> data) = 0;
};

#endif