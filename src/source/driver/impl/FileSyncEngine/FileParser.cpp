#include "driver/impl/FileSyncEngine/FileParser.h"
#include "driver/interface/FileSyncEngine/FileSyncEngineInterface.h"
#include "driver/impl/Nlohmann.h"
#include "control/GlobalStatusManager.h"
#include "driver/impl/FileUtility.h"
#include <string>
#include <iostream>

FileParser::FileParser() :
    json_parser(std::make_unique<NlohmannJson>())
{
    if (!FileSystemUtils::directoryExists(FileSyncEngineInterface::tmp_dir))
    {
        FileSystemUtils::createDirectoryRecursive(FileSyncEngineInterface::tmp_dir);
    }
    type_parser_map["file_header"] = std::bind(&FileParser::onFileHeader, this, std::placeholders::_1);
    type_parser_map["dir_header"] = std::bind(&FileParser::onDirHeader, this, std::placeholders::_1);
    type_parser_map["dir_item_header"] = std::bind(&FileParser::onDirItemHeader, this, std::placeholders::_1);
    type_parser_map["file_end"] = std::bind(&FileParser::onFileEnd, this, std::placeholders::_1);
}

void FileParser::parse(std::unique_ptr<NetworkInterface::UserMsg> msg)
{
    if (msg->header.flag & static_cast<uint8_t>(NetworkInterface::Flag::IS_BINARY))
    {
        if (file_stream)
        {
            std::cout << "data size " << msg->data.size() << std::endl;;
            file_stream->write(reinterpret_cast<const char*>(msg->data.data()) + 4, msg->data.size() - 4);
            std::cout << "File read done" << std::endl;
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
    std::cout << "File Header" << std::endl;
    uint32_t id = std::stol(content_parser->getValue("id"));
    if (file_stream)
    {
        file_stream.release();
    }
    file_stream = std::make_unique<std::ofstream>(FileSyncEngineInterface::tmp_dir + GlobalStatusManager::getInstance().getFileName(id),
        std::ios::binary);
    if (!file_stream->is_open())
    {
        std::cout << "Failed to open" << GlobalStatusManager::getInstance().getFileName(id) << std::endl;
    }
}

void FileParser::onDirHeader(std::unique_ptr<Json::Parser> content_parser)
{
    std::cout << "Dir Header" << std::endl;
}

void FileParser::onDirItemHeader(std::unique_ptr<Json::Parser> content_parser)
{
    std::cout << "Dir Item Header" << std::endl;
}

void FileParser::onFileEnd(std::unique_ptr<Json::Parser> content_parser)
{
    std::cout << "File end" << std::endl;
    file_stream->flush();
    file_stream.reset();
}