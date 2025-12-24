#ifndef FILESENDERINTERFACE_H
#define FILESENDERINTERFACE_H

#include <string>
#include "driver/interface/SecurityInterface.h"
#include "driver/impl/OuterMsgBuilder.h"
#include <condition_variable>
#include <utility>
#include <functional>
#include <optional>
#include <iostream>

class FileSenderInterface
{
public:
    FileSenderInterface(const std::string &addr, const std::string &p, std::shared_ptr<SecurityInterface> inst) : address(addr), port(p), security_instance(inst)
    {
    }
    virtual ~FileSenderInterface() = default;
    virtual bool initialize() = 0;
    virtual void start(std::function<std::optional<std::pair<uint32_t, std::string>>()> get_task_cb) = 0;
    virtual void stop() = 0;
    virtual void setCondition(std::shared_ptr<std::condition_variable> queue_cv) { cv = queue_cv; }
    virtual void setCheckQueue(std::function<bool()> check_cb) { check_queue_cb = check_cb; }
    virtual uint32_t getCurrentFileID() { return current_file_id; }
    virtual void cancelSending() { cancel = true; }

protected:
    static OuterMsgBuilderInterface &getOuterMsgBuilder()
    {
        static OuterMsgBuilder instance;
        return instance;
    }
    std::shared_ptr<SecurityInterface> security_instance;
    std::string address;
    std::string port;
    std::shared_ptr<std::condition_variable> cv;
    std::function<bool()> check_queue_cb;
    bool running{false};
    bool cancel{false};
    uint32_t current_file_id;
};

#endif