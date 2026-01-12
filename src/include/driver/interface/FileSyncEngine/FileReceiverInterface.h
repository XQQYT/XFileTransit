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
    virtual bool initialize(const std::string &addr, const std::string &p, std::shared_ptr<SecurityInterface> inst) = 0;
    virtual void start(std::function<void(UnifiedSocket)> accept_cb,
                       std::function<void(UnifiedSocket socket, std::unique_ptr<NetworkInterface::UserMsg>)> msg_cb) = 0;
    virtual void stop() = 0;
    virtual void closeReceiver() = 0;
    virtual void removeSocket(UnifiedSocket socket) = 0;

protected:
    std::shared_ptr<SecurityInterface> security_instance;
};

#endif