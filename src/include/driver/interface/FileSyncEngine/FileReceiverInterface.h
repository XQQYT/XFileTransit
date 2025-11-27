#ifndef FILERECEIVERINTERFACE_H
#define FILERECEIVERINTERFACE_H

#include "driver/interface/SecurityInterface.h"
#include <string>

class FileReceiverInterface
{
public:
    FileReceiverInterface(const std::string& addr, const std::string& p, std::shared_ptr<SecurityInterface> inst) :
        address(addr), port(p), security_instance(inst) {
    }
    virtual ~FileReceiverInterface() = default;
    virtual bool initialize() = 0;
    virtual void start(std::function<void(uint32_t id, float progress)> progress_cb) = 0;
    virtual void stop() = 0;
protected:
    std::shared_ptr<SecurityInterface> security_instance;
    std::string address;
    std::string port;
    bool running{ false };
};

#endif