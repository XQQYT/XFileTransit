#ifndef _USERSERVERMSG_H
#define _USERSERVERMSG_H

#include "driver/interface/MsgBuilderInterface.h"
#include "driver/interface/SecurityInterface.h"
#include <vector>
#include <memory>
#include <stdint.h>

class MsgBuilder : public MsgBuilderInterface
{
public:
    MsgBuilder(std::shared_ptr<SecurityInterface> instance = nullptr);
    ~MsgBuilder() = default;
    std::unique_ptr<MsgBuilderInterface::UserMsg> buildMsg(std::string payload) override;

private:
    uint8_t version;
};

#endif //_USERSERVERMSG_H