#include "driver/impl/FileSyncEngine/FileMsgBuilder.h"
#include "driver/interface/FileSyncEngine/FileSyncEngineInterface.h"
#include "driver/impl/Nlohmann.h"
#include "driver/impl/FileUtility.h"

FileMsgBuilder::FileMsgBuilder() :
    json_builder(std::make_unique<NlohmannJson>())
{

}

std::unique_ptr<std::vector<uint8_t>> FileMsgBuilder::buildHeader()
{
    if (FileSystemUtils::isDirectory(file_path))//文件夹
    {
        current_state = State::Block;
        return nullptr;
    }
    else//单独文件
    {
        total_size = FileSystemUtils::getFileSize(file_path);
        uint64_t total_blocks = (total_size + FileSyncEngineInterface::file_block_size - 1)
            / FileSyncEngineInterface::file_block_size;
        auto json = json_builder->getBuilder(Json::BuilderType::File);
        std::string json_str = json->buildFileMsg(Json::MessageType::File::FileHeader, {
            {"id",std::to_string(file_id)},
            {"total_size",std::to_string(total_size)},
            {"total_blocks",std::to_string(total_blocks)},
            {"block_size",std::to_string(FileSyncEngineInterface::file_block_size)}
            });
        auto result = std::make_unique<std::vector<uint8_t>>();
        result->reserve(json_str.size());
        std::transform(json_str.begin(), json_str.end(),
            std::back_inserter(*result),
            [](char c) { return static_cast<uint8_t>(c); });
        current_state = State::Block;
        return result;
    }
    return nullptr;
}

std::unique_ptr<std::vector<uint8_t>> FileMsgBuilder::getStream()
{
    if (!is_initialized)
    {
        throw std::runtime_error("File Info haven't be initialized");
    }
    switch (current_state)
    {
    case State::Header:
        return buildHeader();
    case State::Block:
        return nullptr;
    case State::End:
        return nullptr;
    default:
        throw std::runtime_error("Invalid file builder state");
    }
}