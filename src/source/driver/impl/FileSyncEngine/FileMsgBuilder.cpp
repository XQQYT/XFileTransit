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
    if (is_folder && file_state == State::Default)//第一次消息且是文件夹则发送文件夹元信息
    {
        uint32_t total_paths = 0;
        auto leaf_paths = FileSystemUtils::vectorToJsonString(FileSystemUtils::findAllLeafFolders(file_path, total_paths));
        auto json = json_builder->getBuilder(Json::BuilderType::File);
        std::string json_str = json->buildFileMsg(Json::MessageType::File::DirectoryHeader, {
            {"id",std::to_string(file_id)},
            {"leaf_paths",std::move(leaf_paths)},
            {"total_paths",std::to_string(total_paths)}
            });
        auto result = std::make_unique<std::vector<uint8_t>>();
        result->reserve(json_str.size());
        std::transform(json_str.begin(), json_str.end(),
            std::back_inserter(*result),
            [](char c) { return static_cast<uint8_t>(c); });
        file_state = State::Header;
        dir_items = FileSystemUtils::findAllLeafFiles(file_path);
        return result;
    }
    else if (!is_folder && file_state == State::Default)//不是文件夹且是第一次消息，发送文件元消息
    {
        total_size = FileSystemUtils::getFileSize(file_path);
        uint64_t total_blocks = (total_size + FileSyncEngineInterface::file_block_size - 1)
            / FileSyncEngineInterface::file_block_size;
        auto json = json_builder->getBuilder(Json::BuilderType::File);
        std::string json_str = json->buildFileMsg(Json::MessageType::File::FileHeader, {
            {"id",std::to_string(file_id)},
            {"total_size",std::to_string(total_size)},
            {"total_blocks",std::to_string(total_blocks)},
            });
        auto result = std::make_unique<std::vector<uint8_t>>();
        result->reserve(json_str.size());
        std::transform(json_str.begin(), json_str.end(),
            std::back_inserter(*result),
            [](char c) { return static_cast<uint8_t>(c); });
        file_state = State::Block;
        return result;
    }
    else if (is_folder && file_state == State::Header)//是文件夹且需要发送Header，则发送文件项的元信息
    {
        std::string current_file = dir_items[dir_file_index++];
        total_size = FileSystemUtils::getFileSize(current_file);
        uint64_t total_blocks = (total_size + FileSyncEngineInterface::file_block_size - 1)
            / FileSyncEngineInterface::file_block_size;
        auto json = json_builder->getBuilder(Json::BuilderType::File);
        std::string json_str = json->buildFileMsg(Json::MessageType::File::DirectoryItemHeader, {
            {"id",std::to_string(file_id)},
            {"total_size",std::to_string(total_size)},
            {"path",FileSystemUtils::absoluteToRelativePath(current_file, file_path)},
            {"total_blocks",std::to_string(total_blocks)},
            });
        auto result = std::make_unique<std::vector<uint8_t>>();
        result->reserve(json_str.size());
        std::transform(json_str.begin(), json_str.end(),
            std::back_inserter(*result),
            [](char c) { return static_cast<uint8_t>(c); });
        file_state = State::Block;
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
    //初次调用，发送文件头或文件夹头
    if (file_state == State::Default)
    {
        is_folder = FileSystemUtils::isDirectory(file_path);
        return buildHeader();
    }
    auto i = std::make_unique<std::vector<uint8_t>>();
    switch (file_state)
    {
    case State::Header:
        return buildHeader();
    case State::Block:
        file_state = State::End;
        return i;
    case State::End:
        if (dir_file_index <= dir_items.size() - 1)
        {
            file_state = State::Header;
            return buildHeader();
        }
        return nullptr;
    default:
        break;
    }
    return nullptr;

}