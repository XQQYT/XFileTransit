#ifndef FILEMSGBUILDERINTERFACE_H
#define FILEMSGBUILDERINTERFACE_H

#include <memory>
#include <vector>
#include <string>

class FileMsgBuilderInterface
{
public:
    virtual std::unique_ptr<std::vector<uint8_t>> build(uint32_t id, std::string path) = 0;
};

#endif