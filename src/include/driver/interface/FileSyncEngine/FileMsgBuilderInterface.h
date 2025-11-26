#ifndef FILEMSGBUILDERINTERFACE_H
#define FILEMSGBUILDERINTERFACE_H

#include <memory>
#include <vector>

class FileMsgBuilderInterface
{
public:
    virtual std::unique_ptr<std::vector<uint8_t>> build(std::vector<uint8_t>&& file_chunk) = 0;
};

#endif