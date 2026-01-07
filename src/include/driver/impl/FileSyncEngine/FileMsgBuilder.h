#ifndef FILEMSGBUILDER_H
#define FILEMSGBUILDER_H

#include "driver/interface/FileSyncEngine/FileMsgBuilderInterface.h"
#include "driver/interface/JsonFactoryInterface.h"
#include <fstream>

class FileMsgBuilder : public FileMsgBuilderInterface
{
public:
    FileMsgBuilderInterface::FileMsgBuilderResult getStream() override;
    void cancelSending() override;
    FileMsgBuilder();

private:
    std::unique_ptr<std::vector<uint8_t>> buildHeader();
    std::unique_ptr<std::vector<uint8_t>> buildEnd();
    std::unique_ptr<std::vector<uint8_t>> buildCanceled();
    std::unique_ptr<std::vector<uint8_t>> buildBlock();
    uint8_t calculateProgress();

private:
    enum class State
    {
        Default,
        Header,
        Block,
        End,
        Cancel
    };
    State file_state{State::Default};
    uint64_t block_index{0};
    std::unique_ptr<Json::JsonFactoryInterface> json_builder;
    uint64_t file_total_size{0};
    uint64_t file_sended_size{0};
    uint64_t dir_total_size{0};
    uint64_t dir_sended_size{0};
    uint64_t total_blocks{0};
    bool is_folder{false};
    bool is_end{false};
    uint64_t dir_file_index{0};
    std::vector<std::string> dir_items;
    std::unique_ptr<std::ifstream> file_reader;
};

#endif