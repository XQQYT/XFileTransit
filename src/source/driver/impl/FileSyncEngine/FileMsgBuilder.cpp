#include "driver/impl/FileSyncEngine/FileMsgBuilder.h"
#include "driver/impl/Nlohmann.h"

FileMsgBuilder::FileMsgBuilder() :
    json_builder(std::make_unique<NlohmannJson>())
{

}

std::unique_ptr<std::vector<uint8_t>> FileMsgBuilder::build(uint32_t id, std::string path)
{

}