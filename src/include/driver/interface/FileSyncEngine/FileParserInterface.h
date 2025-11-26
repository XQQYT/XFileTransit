#ifndef FILEPARSERINTERFACE_H
#define FILEPARSERINTERFACE_H

#include <vector>
#include <stdint.h>

class FileParserInterface
{
public:
    virtual void parse(std::vector<uint8_t> payload) = 0;
};

#endif