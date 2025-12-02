#include "driver/impl/FileSyncEngine/FileParser.h"
#include "driver/interface/FileSyncEngine/FileSyncEngineInterface.h"
#include "driver/impl/Nlohmann.h"
#include "control/GlobalStatusManager.h"
#include "driver/impl/FileUtility.h"
#include "control/EventBusManager.h"
#include <string>
#include <iostream>

FileParser::FileParser() :
    json_parser(std::make_unique<NlohmannJson>())
{
    if (!FileSystemUtils::directoryExists(FileSyncEngineInterface::tmp_dir))
    {
        FileSystemUtils::createDirectoryRecursive(FileSystemUtils::utf8ToWide(FileSyncEngineInterface::tmp_dir));
    }
    type_parser_map["file_header"] = std::bind(&FileParser::onFileHeader, this, std::placeholders::_1);
    type_parser_map["dir_header"] = std::bind(&FileParser::onDirHeader, this, std::placeholders::_1);
    type_parser_map["dir_item_header"] = std::bind(&FileParser::onDirItemHeader, this, std::placeholders::_1);
    type_parser_map["file_end"] = std::bind(&FileParser::onFileEnd, this, std::placeholders::_1);
}

uint8_t FileParser::calculateProgress()
{
    if (total_size == 0) {
        return 0;
    }
    uint64_t effective_size = (received_size > total_size) ? total_size : received_size;
    return static_cast<uint8_t>((effective_size * 100 + total_size / 2) / total_size);
}

void FileParser::parse(std::unique_ptr<NetworkInterface::UserMsg> msg)
{
    static uint8_t progress_count = 0;
    if (msg->header.flag & static_cast<uint8_t>(NetworkInterface::Flag::IS_BINARY))
    {
        if (file_stream)
        {
            file_stream->write(reinterpret_cast<const char*>(msg->data.data()) + 4, msg->data.size() - 4);
            received_size += msg->data.size() - 4;
            if (progress_count >= 30)
            {
                EventBusManager::instance().publish("/file/download_progress", current_file_id, calculateProgress(), false);
                progress_count = 0;
            }
            ++progress_count;
        }
        else
        {
            std::cout << "FStream hasn't ready" << std::endl;
        }
    }
    else
    {
        auto parser = json_parser->getParser();
        std::string json_str = std::string(msg->data.data(), msg->data.data() + msg->data.size());
        parser->loadJson(json_str);
        std::cout << "File Msg  " << std::string(msg->data.data(), msg->data.data() + msg->data.size()) << std::endl;
        auto parser_func = type_parser_map.find(parser->getValue("type"));
        if (parser_func != type_parser_map.end())
        {
            parser_func->second(parser->getObj("content"));
        }
        else
        {
            std::cout << "Invalid type" << std::endl;
        }
    }
}

void FileParser::onFileHeader(std::unique_ptr<Json::Parser> content_parser)
{
    uint32_t id = std::stol(content_parser->getValue("id"));
    if (file_stream)
    {
        file_stream.release();
    }
    //接收到的字符是utf8，需要转换成宽字节
    std::wstring wide_tmp_dir = FileSystemUtils::utf8ToWide(FileSyncEngineInterface::tmp_dir);
    std::wstring wide_filename = FileSystemUtils::utf8ToWide(GlobalStatusManager::getInstance().getFileName(id));
    std::wstring full_path = wide_tmp_dir + wide_filename;

    current_file_id = std::stol(content_parser->getValue("id"));
    file_stream = std::make_unique<std::ofstream>(full_path.c_str(), std::ios::binary);
    total_size = std::stol(content_parser->getValue("total_size"));
    if (!file_stream->is_open())
    {
        std::wcout << L"Failed to open: " << full_path << std::endl;
    }
}

void FileParser::onDirHeader(std::unique_ptr<Json::Parser> content_parser)
{
    uint32_t id = std::stol(content_parser->getValue("id"));
    total_size = std::stol(content_parser->getValue("total_size"));
    std::wstring wide_tmp_dir = FileSystemUtils::utf8ToWide(FileSyncEngineInterface::tmp_dir);
    std::wstring wide_filename = FileSystemUtils::utf8ToWide(GlobalStatusManager::getInstance().getFileName(id));
    std::wstring end = FileSystemUtils::utf8ToWide("/");
    dir_path = wide_tmp_dir + wide_filename + end;
    auto leaf_paths = content_parser->getArray("leaf_paths");
    for (auto& i : leaf_paths)
    {
        auto tmp = i->getArrayItems();
        for (auto& a : tmp)
        {
            FileSystemUtils::createDirectoryRecursive(dir_path + FileSystemUtils::utf8ToWide(a));
        }
    }
    current_file_id = std::stol(content_parser->getValue("id"));
    is_folder = true;
}

void FileParser::onDirItemHeader(std::unique_ptr<Json::Parser> content_parser)
{
    std::wstring file_relative_path = FileSystemUtils::utf8ToWide(content_parser->getValue("path"));
    std::wstring full_path = dir_path + file_relative_path;
    file_stream = std::make_unique<std::ofstream>(full_path.c_str(), std::ios::binary);
    if (!file_stream->is_open())
    {
        std::wcout << L"Failed to open: " << full_path << std::endl;
    }
    file_name = content_parser->getValue("path");
}

void FileParser::onFileEnd(std::unique_ptr<Json::Parser> content_parser)
{
    EventBusManager::instance().publish("/file/download_progress", current_file_id, static_cast<uint8_t>(100), true);
    received_size = 0;
    progress_count = 0;
    file_stream->flush();
    file_stream.reset();
}