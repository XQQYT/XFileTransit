#ifndef _USERSERVERMSG_H
#define _USERSERVERMSG_H

#include "driver/interface/OuterMsgBuilderInterface.h"
#include "driver/interface/SecurityInterface.h"
#include <vector>
#include <memory>
#include <stdint.h>

class OuterMsgBuilder : public OuterMsgBuilderInterface
{
public:
    OuterMsgBuilder(std::shared_ptr<SecurityInterface> instance = nullptr);
    ~OuterMsgBuilder() = default;
    std::unique_ptr<NetworkInterface::UserMsg> buildMsg(std::string payload, NetworkInterface::Flag flag) override;
    std::unique_ptr<NetworkInterface::UserMsg> buildMsg(std::vector<uint8_t> payload, NetworkInterface::Flag flag) override;
private:
    std::unique_ptr<NetworkInterface::UserMsg> build(std::vector<uint8_t> payload, NetworkInterface::Flag flag) override;
    uint8_t version;
};

#endif //_USERSERVERMSG_H