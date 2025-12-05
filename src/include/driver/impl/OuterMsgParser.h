#ifndef OUTERMSGPARSER_H
#define OUTERMSGPARSER_H

#include "driver/interface/OuterMsgParserInterface.h"

class OuterMsgParser : public OuterMsgParserInterface
{
public:
    void delegateRecv(SOCKET client_socket,
        std::function<void(std::unique_ptr<NetworkInterface::UserMsg> parsed_msg)> callback,
        std::function<void()> dcc_cb,
        std::function<void(const NetworkInterface::RecvError error)> dre_cb,
        std::shared_ptr<SecurityInterface> security_instance,
        bool& running) override;
private:
    void dealRecvError(std::function<void()> dcc_cb,
        std::function<void(const NetworkInterface::RecvError error)> dre_cb);
    std::unique_ptr<NetworkInterface::UserMsg> parse(std::vector<uint8_t>&& msg, const uint32_t length, const uint8_t flag) override;
};

#endif