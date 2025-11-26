#ifndef _MSGBUILDERINTERFACE_H
#define _MSGBUILDERINTERFACE_H

#include <string>
#include <vector>
#include <memory>
#include <string>
#include "driver/interface/SecurityInterface.h"
#include "driver/interface/NetworkInterface.h"

class OuterMsgBuilderInterface
{
public:
    virtual ~OuterMsgBuilderInterface() {};
    virtual std::unique_ptr<NetworkInterface::UserMsg> buildMsg(std::string payload) = 0;
    virtual std::unique_ptr<NetworkInterface::UserMsg> buildMsg(std::vector<uint8_t> payload) = 0;
    virtual void setSecurityInstance(std::shared_ptr<SecurityInterface> instance) { security_instance = instance; }
private:
    virtual std::unique_ptr<NetworkInterface::UserMsg> build(std::vector<uint8_t> payload) = 0;
protected:
    std::shared_ptr<SecurityInterface> security_instance;
};


#endif //_MSGBUILDERINTERFACE_H