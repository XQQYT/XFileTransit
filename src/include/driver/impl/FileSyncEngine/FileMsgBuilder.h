#ifndef FILEMSGBUILDER_H
#define FILEMSGBUILDER_H

#include "driver/interface/FileSyncEngine/FileMsgBuilderInterface.h"
#include "driver/interface/JsonFactoryInterface.h"
#include <fstream>

class FileMsgBuilder : public FileMsgBuilderInterface
{
public:
    std::pair<bool, std::unique_ptr<std::vector<uint8_t>>> getStream() override;
    FileMsgBuilder();
private:
    std::unique_ptr<std::vector<uint8_t>> buildHeader();
    std::unique_ptr<std::vector<uint8_t>> buildEnd();
    std::unique_ptr<std::vector<uint8_t>> buildBlock();
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
    uint64_t total_size{ 0 };
    uint64_t total_blocks{ 0 };
    bool is_folder{ false };
    bool is_end{ false };
    uint64_t dir_file_index{ 0 };
    std::vector<std::string> dir_items;
    std::unique_ptr<std::ifstream> file_reader;
    uint64_t readed_size{ 0 };
};

#endif