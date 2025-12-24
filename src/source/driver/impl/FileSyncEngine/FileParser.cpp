#include "driver/impl/FileSyncEngine/FileParser.h"
#include "driver/interface/FileSyncEngine/FileSyncEngineInterface.h"
#include "driver/impl/Nlohmann.h"
#include "control/GlobalStatusManager.h"
#include "driver/impl/FileUtility.h"
#include "control/EventBusManager.h"
#include "driver/interface/FileStreamHelper.h"
#include "common/DebugOutputer.h"
#include <string>

FileParser::FileParser() : json_parser(std::make_unique<NlohmannJson>())
{
    if (!FileSystemUtils::directoryExists(GlobalStatusManager::absolute_tmp_dir))
    {
        FileSystemUtils::createDirectoryRecursive(GlobalStatusManager::absolute_tmp_dir);
    }
    type_parser_map["file_header"] = std::bind(&FileParser::onFileHeader, this, std::placeholders::_1);
    type_parser_map["dir_header"] = std::bind(&FileParser::onDirHeader, this, std::placeholders::_1);
    type_parser_map["dir_item_header"] = std::bind(&FileParser::onDirItemHeader, this, std::placeholders::_1);
    type_parser_map["file_end"] = std::bind(&FileParser::onFileEnd, this, std::placeholders::_1);
    type_parser_map["file_cancel"] = std::bind(&FileParser::onFileCancel, this, std::placeholders::_1);
}

uint8_t FileParser::calculateProgress()
{
    if (total_size == 0)
    {
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
            file_stream->write(reinterpret_cast<const char *>(msg->data.data()) + 4, msg->data.size() - 4);
            received_size += msg->data.size() - 4;
            bytes_received += msg->data.size() - 4;
            if (progress_count >= 40)
            {
                end_time_point = std::chrono::steady_clock::now();
                auto elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(end_time_point - start_time_point);
                uint32_t speed_bps = 0;
                if (elapsed_us.count() > 0)
                {
                    uint64_t bps = (static_cast<uint64_t>(bytes_received) * 1000000ULL) / elapsed_us.count();
                    speed_bps = static_cast<uint32_t>(bps);
                    bytes_received = 0;
                }
                start_time_point = std::chrono::steady_clock::now();
                EventBusManager::instance().publish("/file/download_progress", current_file_id,
                                                    calculateProgress(), static_cast<uint32_t>(speed_bps), false);
                progress_count = 0;
            }
            ++progress_count;
        }
        else
        {
            LOG_ERROR("FStream hasn't ready");
        }
    }
    else
    {
        auto parser = json_parser->getParser();
        std::string json_str = std::string(msg->data.data(), msg->data.data() + msg->data.size());
        parser->loadJson(json_str);
        LOG_INFO("File Msg  " << std::string(msg->data.data(), msg->data.data() + msg->data.size()));
        auto parser_func = type_parser_map.find(parser->getValue("type"));
        if (parser_func != type_parser_map.end())
        {
            parser_func->second(parser->getObj("content"));
        }
        else
        {
            LOG_ERROR("Invalid type");
        }
    }
}

void FileParser::onFileHeader(std::unique_ptr<Json::Parser> content_parser)
{
    uint32_t id = std::stoul(content_parser->getValue("id"));
    if (file_stream)
    {
        file_stream.reset();
    }

    // 接收到的字符是utf8，需要转换成宽字节
    std::wstring wide_tmp_dir = FileSystemUtils::utf8ToWide(GlobalStatusManager::absolute_tmp_dir);
    std::wstring wide_filename = FileSystemUtils::utf8ToWide(GlobalStatusManager::getInstance().getFileName(id));
    std::wstring full_path = wide_tmp_dir + wide_filename;

    file_path = FileStreamHelper::wstringToLocalPath(full_path);

    current_file_id = std::stoul(content_parser->getValue("id"));

    file_stream = FileStreamHelper::createOutputFileStream(full_path);

    total_size = std::stoull(content_parser->getValue("total_size"));

    if (!file_stream || !file_stream->is_open())
    {
        LOG_ERROR("Failed to open: " << FileStreamHelper::wstringToLocalPath(full_path));
    }
    else
    {
        start_time_point = std::chrono::steady_clock::now();
    }
    is_folder = false;
}

void FileParser::onDirHeader(std::unique_ptr<Json::Parser> content_parser)
{
    uint32_t id = std::stoul(content_parser->getValue("id"));
    total_size = std::stoull(content_parser->getValue("total_size"));
    std::wstring wide_tmp_dir = FileSystemUtils::utf8ToWide(GlobalStatusManager::absolute_tmp_dir);
    std::wstring wide_filename = FileSystemUtils::utf8ToWide(GlobalStatusManager::getInstance().getFileName(id));
    std::wstring end = FileSystemUtils::utf8ToWide("/");
    w_dir_path = wide_tmp_dir + wide_filename + end;

    auto leaf_paths = content_parser->getArray("leaf_paths");
    for (auto &i : leaf_paths)
    {
        auto tmp = i->getArrayItems();
        for (auto &a : tmp)
        {
#ifdef __linux__
            std::replace(a.begin(), a.end(), '\\', '/');
#endif
            std::string path_str = FileStreamHelper::wstringToLocalPath(w_dir_path + FileSystemUtils::utf8ToWide(a));

            FileSystemUtils::createDirectoryRecursive(path_str);
        }
    }

    current_file_id = std::stoul(content_parser->getValue("id"));
    is_folder = true;
    start_time_point = std::chrono::steady_clock::now();
}

void FileParser::onDirItemHeader(std::unique_ptr<Json::Parser> content_parser)
{
    std::string arg_path = content_parser->getValue("path");
#ifdef __linux__
    std::replace(arg_path.begin(), arg_path.end(), '\\', '/');
#endif
    std::wstring file_relative_path = FileSystemUtils::utf8ToWide(arg_path);
    std::wstring full_path = w_dir_path + file_relative_path;

    file_stream = FileStreamHelper::createOutputFileStream(full_path);

    if (!file_stream || !file_stream->is_open())
    {
        LOG_ERROR("Failed to open: " << FileStreamHelper::wstringToLocalPath(full_path));
    }

    uint64_t item_size = std::stoull(content_parser->getValue("total_size"));
    if (item_size <= 0)
    {
        file_stream->flush();
        file_stream.reset();
        return;
    }
}

void FileParser::onFileEnd(std::unique_ptr<Json::Parser> content_parser)
{
    EventBusManager::instance().publish("/file/download_progress", current_file_id,
                                        static_cast<uint8_t>(100), static_cast<uint32_t>(0), true);
    received_size = 0;

    static uint8_t progress_count = 0;
    progress_count = 0;

    if (file_stream)
    {
        file_stream->flush();
        file_stream.reset();
    }
}

void FileParser::onFileCancel(std::unique_ptr<Json::Parser> content_parser)
{
    EventBusManager::instance().publish("/file/have_cancel_transit", current_file_id);
    received_size = 0;

    static uint8_t progress_count = 0;
    progress_count = 0;

    if (file_stream)
    {
        file_stream->flush();
        file_stream.reset();
        if (is_folder)
            FileSystemUtils::removeFileOrDirectory(FileStreamHelper::wstringToLocalPath(w_dir_path), true);
        else
            FileSystemUtils::removeFileOrDirectory(file_path, true);
    }
}
