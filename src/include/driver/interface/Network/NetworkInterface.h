#ifndef _NETWORK_H
#define _NETWORK_H

#include <memory>
#include <functional>
#include <string>
#include "driver/interface/SecurityInterface.h"

class OuterMsgParserInterface;
class NetworkInterface
{
public:
#pragma pack(push, 1)
    struct Header
    {
        uint16_t magic;
        uint8_t version;
        uint32_t length;
        uint8_t flag;
    };
#pragma pack(pop)

    struct UserMsg
    {
        std::vector<uint8_t> iv;
        std::vector<uint8_t> data;
        std::vector<uint8_t> sha256;
        Header header;
    };
    static const uint16_t magic = 0xABCD;
    static const uint8_t version = 0x01;

    enum class Flag : uint8_t
    {
        IS_BINARY = 1 << 0,
        IS_ENCRYPT = 1 << 1
    };

    friend Flag operator|(Flag lhs, Flag rhs)
    {
        return static_cast<Flag>(
            static_cast<std::underlying_type_t<Flag>>(lhs) |
            static_cast<std::underlying_type_t<Flag>>(rhs));
    }

    friend bool operator&(Flag lhs, Flag rhs)
    {
        return static_cast<bool>(
            static_cast<std::underlying_type_t<Flag>>(lhs) &
            static_cast<std::underlying_type_t<Flag>>(rhs));
    }

public:
    NetworkInterface() = default;
    NetworkInterface(const NetworkInterface &obj) = delete;
    NetworkInterface(NetworkInterface &&obj) = delete;
    NetworkInterface &operator=(NetworkInterface &other) = delete;
    NetworkInterface &operator=(NetworkInterface &&other) = delete;
    virtual ~NetworkInterface() = default;

    virtual void connect(const std::string &address, const std::string &port, std::function<void(bool)> callback = nullptr) = 0;
    virtual void startListen(const std::string &address, const std::string &port, std::function<bool(bool)> callback) = 0;
    virtual void sendMsg(const std::string &msg) = 0;
    virtual void recvMsg(std::function<void(std::unique_ptr<UserMsg>)> callback) {};
    virtual void recvMsg(std::function<void(std::string)> callback) {}
    virtual void closeSocket() = 0;
    virtual void resetConnection() = 0;
};

#endif