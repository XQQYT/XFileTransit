#ifndef FILESENDERINTERFACE_H
#define FILESENDERINTERFACE_H

#include <string>
#include "driver/interface/SecurityInterface.h"
#include <condition_variable>
#include <utility>

class FileSenderInterface
{
public:
    FileSenderInterface(const std::string& addr, const std::string& p, std::shared_ptr<SecurityInterface> inst):
    address(addr), port(p), security_instance(inst){}
    virtual ~FileSenderInterface() = default;
    virtual bool initialize() = 0;
    virtual void start(std::function<std::pair<uint32_t,std::string>()> get_task_cb) = 0;
    virtual void stop() = 0;
    virtual void setCondition(std::shared_ptr<std::condition_variable> queue_cv){ cv = queue_cv; }
protected:
    std::shared_ptr<SecurityInterface> security_instance;
    std::string address;
    std::string port;
    std::shared_ptr<std::condition_variable> cv;
    bool running {false};
};

#endif