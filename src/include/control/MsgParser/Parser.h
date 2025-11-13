#ifndef _PARSER_H
#define _PARSER_H

#include <vector>
#include <cstdint>

class Parser
{
public:
    virtual void parse(std::vector<uint8_t>&& data) = 0;
};

#endif