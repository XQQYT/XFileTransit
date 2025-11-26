#ifndef OUTERMSGPARSERINTERFACE_H
#define OUTERMSGPARSERINTERFACE_H

#include <functional>
#include <vector>
#include <memory>

#include "driver/interface/SecurityInterface.h"
#include "driver/interface/NetworkInterface.h"

class OuterMsgParserInterface
{
public:
    virtual void delegateRecv(SOCKET client_socket,
        std::function<void(std::unique_ptr<NetworkInterface::UserMsg> parsed_msg)> callback,
        std::function<void()> dcc_cb,
        std::function<void(const NetworkInterface::RecvError error)> dre_cb,
        std::shared_ptr<SecurityInterface> security_instance) = 0;
private:
    virtual std::unique_ptr<NetworkInterface::UserMsg> parse(std::vector<uint8_t>&& msg, const uint32_t length, const uint8_t flag) = 0;
};


#endif