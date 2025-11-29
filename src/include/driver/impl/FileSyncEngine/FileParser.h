#ifndef FILEPARSER_H
#define FILEPARSER_H

#include "driver/interface/FileSyncEngine/FileParserInterface.h"

class FileParser : public FileParserInterface
{
public:
    void parse(std::unique_ptr<NetworkInterface::UserMsg> msg) override;
};

#endif