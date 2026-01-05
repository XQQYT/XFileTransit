#ifndef FILERECEIVERINTERFACE_H
#define FILERECEIVERINTERFACE_H

#include "driver/interface/SecurityInterface.h"
#include "driver/interface/Network/TcpInterface.h"
#include <string>

class FileReceiverInterface
{
public:
    FileReceiverInterface() = default;
    virtual ~FileReceiverInterface() = default;
    virtual bool initialize(const std::string &addr, const std::string &p, std::shared_ptr<SecurityInterface> inst) { return false; }
    virtual void start(std::function<void(UnifiedSocket)> accept_cb,
                       std::function<void(UnifiedSocket socket, std::unique_ptr<NetworkInterface::UserMsg>)> msg_cb) = 0;
    virtual void stop() {}

protected:
    std::shared_ptr<SecurityInterface> security_instance;
    std::string address;
    std::string port;
    bool running{false};
};

#endif