#ifndef _MSGBUILDERINTERFACE_H
#define _MSGBUILDERINTERFACE_H

#include <string>
#include <vector>
#include <memory>
#include <string>
#include "driver/interface/SecurityInterface.h"
#include "driver/interface/Network/NetworkInterface.h"

class OuterMsgBuilderInterface
{
public:
    virtual ~OuterMsgBuilderInterface() = default;
    virtual std::unique_ptr<NetworkInterface::UserMsg> buildMsg(std::string payload, NetworkInterface::Flag flag) = 0;
    virtual std::unique_ptr<NetworkInterface::UserMsg> buildMsg(std::vector<uint8_t> payload, NetworkInterface::Flag flag) = 0;
    virtual void setSecurityInstance(std::shared_ptr<SecurityInterface> instance) { security_instance = instance; }
    inline static bool encrptyed{true};

private:
    virtual std::unique_ptr<NetworkInterface::UserMsg> build(std::vector<uint8_t> payload, NetworkInterface::Flag flag) = 0;

protected:
    std::shared_ptr<SecurityInterface> security_instance;
};

#endif //_MSGBUILDERINTERFACE_H