#ifndef FILEMSGBUILDERINTERFACE_H
#define FILEMSGBUILDERINTERFACE_H

#include <memory>
#include <vector>
#include <string>

class FileMsgBuilderInterface
{
public:
    virtual void setFileInfo(uint32_t id, const std::string& path) { file_id = id; file_path = path; is_initialized = true; }
    virtual std::pair<bool, std::unique_ptr<std::vector<uint8_t>>> getStream() = 0;
protected:
    uint32_t file_id;
    std::string file_path;
    bool is_initialized{ false };
};

#endif