#include "driver/impl/FileSyncEngine/FileMsgBuilder.h"
#include "driver/interface/FileSyncEngine/FileSyncEngineInterface.h"
#include "driver/impl/Nlohmann.h"
#include "driver/impl/FileUtility.h"

#include <algorithm>

FileMsgBuilder::FileMsgBuilder() :
    json_builder(std::make_unique<NlohmannJson>())
{

}

std::unique_ptr<std::vector<uint8_t>> FileMsgBuilder::buildHeader()
{
    if (is_folder && file_state == State::Default)//第一次消息且是文件夹则发送文件夹元信息
    {
        uint32_t total_paths = 0;
        dir_total_size = FileSystemUtils::calculateFolderSize(file_path);
        auto leaf_paths = FileSystemUtils::vectorToJsonString(FileSystemUtils::findAllLeafFolders(file_path, total_paths));
        auto json = json_builder->getBuilder(Json::BuilderType::File);
        std::string json_str = json->buildFileMsg(Json::MessageType::File::DirectoryHeader, {
            {"id",std::to_string(file_id)},
            {"leaf_paths",std::move(leaf_paths)},
            {"total_paths",std::to_string(total_paths)},
            {"total_size", std::to_string(dir_total_size)}
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
        file_reader = std::make_unique<std::ifstream>(FileSystemUtils::utf8ToWide(file_path).c_str(), std::ios::binary);
        if (!file_reader->is_open())
        {
            std::error_code ec(errno, std::generic_category());
            std::cout << "Failed to open file: " << ec.message() << std::endl;
        }
        file_total_size = FileSystemUtils::getFileSize(file_path);
        uint64_t total_blocks = (file_total_size + FileSyncEngineInterface::file_block_size - 1)
            / FileSyncEngineInterface::file_block_size;
        auto json = json_builder->getBuilder(Json::BuilderType::File);
        std::string json_str = json->buildFileMsg(Json::MessageType::File::FileHeader, {
            {"id",std::to_string(file_id)},
            {"total_size",std::to_string(file_total_size)},
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
        file_reader = std::make_unique<std::ifstream>(FileSystemUtils::utf8ToWide(current_file).c_str(), std::ios::binary);
        if (file_reader->is_open())
        {
            std::cout << "Failed to open file" << std::endl;
        }
        file_total_size = FileSystemUtils::getFileSize(current_file);
        uint64_t total_blocks = (file_total_size + FileSyncEngineInterface::file_block_size - 1)
            / FileSyncEngineInterface::file_block_size;
        auto json = json_builder->getBuilder(Json::BuilderType::File);
        std::string json_str = json->buildFileMsg(Json::MessageType::File::DirectoryItemHeader, {
            {"id",std::to_string(file_id)},
            {"total_size",std::to_string(file_total_size)},
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


std::unique_ptr<std::vector<uint8_t>> FileMsgBuilder::buildBlock()
{
    constexpr uint32_t HEADER_SIZE = sizeof(file_id);
    uint32_t offset = 0;

    // 计算本次可读取的数据大小
    uint64_t remaining_data = file_total_size - file_sended_size;
    uint64_t max_data_size = FileSyncEngineInterface::file_block_size - HEADER_SIZE;
    uint64_t ready_to_read_size = std::min(remaining_data, max_data_size);

    // 创建数据块（实际大小 = 头部 + 数据）
    uint64_t total_block_size = HEADER_SIZE + ready_to_read_size;
    auto result = std::make_unique<std::vector<uint8_t>>(total_block_size);

    // 写入文件 ID 头部
    memcpy(result->data() + offset, &file_id, HEADER_SIZE);
    offset += HEADER_SIZE;

    // 读取文件数据
    if (ready_to_read_size > 0) {
        file_reader->read(reinterpret_cast<char*>(result->data() + offset),
            ready_to_read_size);

        // 检查实际读取的字节数
        std::streamsize bytes_read = file_reader->gcount();
        if (is_folder)
        {
            dir_sended_size += bytes_read;
        }
        file_sended_size += bytes_read;

        // 如果读取的字节数少于预期，调整向量大小
        if (bytes_read < static_cast<std::streamsize>(ready_to_read_size)) {
            result->resize(HEADER_SIZE + bytes_read);
        }

        // 检查是否到达文件末尾
        if (file_reader->eof() || file_sended_size == file_total_size) {
            file_state = State::End;
        }
    }
    return result;
}

std::unique_ptr<std::vector<uint8_t>> FileMsgBuilder::buildEnd()
{
    auto json = json_builder->getBuilder(Json::BuilderType::File);
    std::string json_str = json->buildFileMsg(Json::MessageType::File::FileEnd, {});
    auto result = std::make_unique<std::vector<uint8_t>>();
    result->reserve(json_str.size());
    std::transform(json_str.begin(), json_str.end(),
        std::back_inserter(*result),
        [](char c) { return static_cast<uint8_t>(c); });
    file_state = State::Default;
    return result;
}

uint8_t FileMsgBuilder::calculateProgress()
{
    if (is_folder)
    {
        if (dir_total_size == 0) {
            return 0;
        }

        uint64_t effective_size = (file_sended_size > dir_total_size) ? dir_total_size : file_sended_size;

        return static_cast<uint8_t>((effective_size * 100 + dir_total_size / 2) / dir_total_size);
    }
    else
    {
        if (file_total_size == 0) {
            return 0;
        }

        uint64_t effective_size = (file_sended_size > file_total_size) ? file_total_size : file_sended_size;

        return static_cast<uint8_t>((effective_size * 100 + file_total_size / 2) / file_total_size);
    }

}
FileMsgBuilderInterface::FileMsgBuilderResult FileMsgBuilder::getStream()
{
    if (!is_initialized)
    {
        throw std::runtime_error("File Info haven't be initialized");
    }
    if (is_end)//进入End状态
    {
        is_end = false;//重置
        return { false, 100, nullptr };
    }
    //初次调用，发送文件头或文件夹头
    if (file_state == State::Default)
    {
        is_folder = FileSystemUtils::isDirectory(file_path);
        return { false, 0, buildHeader() };
    }
    switch (file_state)
    {
    case State::Header:
        return { false, 0, buildHeader() };
    case State::Block:
        return { true, calculateProgress(),buildBlock() };
    case State::End:
        if (is_folder && dir_file_index <= dir_items.size() - 1)
        {
            file_state = State::Header;
            return { false, 0, buildHeader() };
        }
        file_state = State::Default;
        is_end = true;
        return { false, 100, buildEnd() };
    default:
        break;
    }
    return { false, 0 ,nullptr };

}