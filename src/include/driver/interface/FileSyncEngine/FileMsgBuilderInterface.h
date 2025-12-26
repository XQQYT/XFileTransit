#ifndef FILEMSGBUILDERINTERFACE_H
#define FILEMSGBUILDERINTERFACE_H

#include <memory>
#include <vector>
#include <string>

class FileMsgBuilderInterface
{
public:
    struct FileMsgBuilderResult
    {
        bool is_binary;
        uint8_t progress;
        std::unique_ptr<std::vector<uint8_t>> data;
    };
    virtual void setFileInfo(uint32_t id, const std::string &path)
    {
        file_id = id;
        file_path = path;
        is_initialized = true;
    }
    virtual FileMsgBuilderResult getStream() = 0;
    virtual void cancelSending() = 0;
    virtual ~FileMsgBuilderInterface() = default;

protected:
    uint32_t file_id;
    std::string file_path;
    bool is_initialized{false};
};

#endif