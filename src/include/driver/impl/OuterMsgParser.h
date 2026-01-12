#ifndef OUTERMSGPARSER_H
#define OUTERMSGPARSER_H

#include "driver/interface/OuterMsgParserInterface.h"

class OuterMsgParser : public OuterMsgParserInterface
{
public:
    std::unique_ptr<NetworkInterface::UserMsg> parse(std::vector<uint8_t> &&msg, const uint32_t length, const uint8_t flag) override;
    void dealRecvError(std::function<void()> dcc_cb,
                       std::function<void(const TcpInterface::RecvError error)> dre_cb) override;
};

#endif