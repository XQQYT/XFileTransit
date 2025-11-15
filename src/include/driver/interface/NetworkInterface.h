#ifndef _NETWORK_H
#define _NETWORK_H

#include <memory>
#include <functional>
#include "driver/interface/SecurityInterface.h"
#include "driver/interface/MsgBuilderInterface.h"

class NetworkInterface {
public:
    struct ParsedMsg {
        std::vector<uint8_t> iv;
        std::vector<uint8_t> data;
        std::vector<uint8_t> sha256;
        MsgBuilderInterface::Header header;
    };
public:
    NetworkInterface() {};
    NetworkInterface(const NetworkInterface& obj) = delete;
    NetworkInterface(NetworkInterface&& obj) = delete;
    NetworkInterface& operator=(NetworkInterface& other) = delete;
    NetworkInterface& operator=(NetworkInterface&& other) = delete;
    virtual ~NetworkInterface() {};
    virtual void initSocket(const std::string& address, const std::string& tls_port, const std::string& tcp_port) = 0;
    virtual void connectTo(std::function<void(bool)> callback = nullptr) = 0;
    virtual void startListen(const std::string& address, const std::string& tls_port, const std::string& tcp_port,
        std::function<bool(bool)> tls_callback,std::function<bool(bool)> tcp_callback) = 0;
    virtual void sendMsg(const std::string& msg) = 0;
    virtual void recvMsg(std::function<void(ParsedMsg&&)> callback) = 0;
    virtual void closeSocket() = 0;
    virtual void resetConnection() = 0;
    virtual void setSecurityInstance(std::shared_ptr<SecurityInterface> instance) { security_instance = instance; }
protected:
    std::shared_ptr<SecurityInterface> security_instance;
};

#endif