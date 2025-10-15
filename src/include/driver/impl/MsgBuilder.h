#ifndef _USERSERVERMSG_H
#define _USERSERVERMSG_H

#include "driver/interface/MsgBuilderInterface.h"
#include "driver/interface/SecurityInterface.h"
#include <vector>
#include <memory>
#include <stdint.h>

static const uint16_t magic = 0xABCD;
static const uint8_t version = 0x01;

#pragma pack(push, 1)
struct  Header
{
    uint16_t magic;
    uint8_t version;
    uint32_t length;
};
#pragma pack(pop)

class MsgBuilder : public MsgBuilderInterface
{
public:
    MsgBuilder(std::shared_ptr<SecurityInterface> instance = nullptr);
    ~MsgBuilder() = default;
    std::unique_ptr<MsgBuilderInterface::UserMsg> buildMsg(std::string payload, const uint8_t* key) override;
    std::unique_ptr<MsgBuilderInterface::UserMsg> buildFile(MsgBuilderInterface::MessageType type, std::string username, std::string path, uint8_t* key) {};

private:
    uint8_t version;
};

#endif //_USERSERVERMSG_H