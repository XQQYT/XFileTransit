#ifndef FILEMSGBUILDER_H
#define FILEMSGBUILDER_H

#include "driver/interface/FileSyncEngine/FileMsgBuilderInterface.h"
#include "driver/interface/JsonFactoryInterface.h"

class FileMsgBuilder : public FileMsgBuilderInterface
{
public:
    std::unique_ptr<std::vector<uint8_t>> getStream() override;
    FileMsgBuilder();
private:
    enum class State
    {
        Default,
        Header,
        Block,
        End
    };
    State file_state{ State::Default };
    uint64_t block_index{ 0 };
    std::unique_ptr<Json::JsonFactoryInterface> json_builder;
private:
    std::unique_ptr<std::vector<uint8_t>> buildHeader();
    uint64_t total_size{ 0 };
    uint64_t total_blocks{ 0 };
    bool is_folder{ false };
    uint64_t dir_file_index{ 0 };
    std::vector<std::string> dir_items;
};

#endif