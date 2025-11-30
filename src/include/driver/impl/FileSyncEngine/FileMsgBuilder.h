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
        Header,
        Block,
        End
    };
    State current_state{ State::Header };
    uint64_t block_index{ 0 };
    std::unique_ptr<Json::JsonFactoryInterface> json_builder;
private:
    std::unique_ptr<std::vector<uint8_t>> buildHeader();
    uint64_t total_size{ 0 };
    uint64_t total_blocks{ 0 };
};

#endif