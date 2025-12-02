#ifndef FILEPARSER_H
#define FILEPARSER_H

#include "driver/interface/FileSyncEngine/FileParserInterface.h"
#include "driver/interface/JsonFactoryInterface.h"
#include <map>
#include <fstream>

class FileParser : public FileParserInterface
{
public:
    FileParser();
    void parse(std::unique_ptr<NetworkInterface::UserMsg> msg) override;
private:
    void onFileHeader(std::unique_ptr<Json::Parser> content_parser);
    void onDirHeader(std::unique_ptr<Json::Parser> content_parser);
    void onDirItemHeader(std::unique_ptr<Json::Parser> content_parser);
    void onFileEnd(std::unique_ptr<Json::Parser> content_parser);
    uint8_t calculateProgress();
private:
    std::unique_ptr<Json::JsonFactoryInterface> json_parser;
    std::map < std::string, std::function<void(std::unique_ptr<Json::Parser>)>> type_parser_map;
    std::unique_ptr<std::ofstream> file_stream;
    std::wstring dir_path;
    uint32_t current_file_id;
    uint64_t total_size{ 0 };
    uint64_t received_size{ 0 };
    bool is_folder{ false };
    uint8_t progress_count{ 0 };
    std::string file_name;
};

#endif