#ifndef OUTERMSGPARSERINTERFACE_H
#define OUTERMSGPARSERINTERFACE_H

#include <functional>
#include <vector>
#include <memory>

#include "driver/interface/SecurityInterface.h"
#include "driver/interface/Network/NetworkInterface.h"
#include "driver/interface/Network/TcpInterface.h"

class OuterMsgParserInterface
{
public:
    virtual std::unique_ptr<NetworkInterface::UserMsg> parse(std::vector<uint8_t> &&msg, const uint32_t length, const uint8_t flag) = 0;
    virtual void dealRecvError(std::function<void()> dcc_cb,
                               std::function<void(const TcpInterface::RecvError error)> dre_cb) = 0;
    virtual ~OuterMsgParserInterface() = default;
};

#endif