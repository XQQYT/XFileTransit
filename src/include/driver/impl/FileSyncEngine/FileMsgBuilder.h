#ifndef FILEMSGBUILDER_H
#define FILEMSGBUILDER_H

#include "driver/interface/FileSyncEngine/FileMsgBuilderInterface.h"
#include "driver/interface/JsonFactoryInterface.h"

class FileMsgBuilder : public FileMsgBuilderInterface
{
public:
    FileMsgBuilder();
    std::unique_ptr<std::vector<uint8_t>> build(uint32_t id, std::string path) override;
private:
    std::unique_ptr<Json::JsonFactoryInterface> json_builder;
};

#endif