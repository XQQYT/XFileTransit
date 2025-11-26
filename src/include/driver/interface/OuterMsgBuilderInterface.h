#ifndef _MSGBUILDERINTERFACE_H
#define _MSGBUILDERINTERFACE_H

#include <string>
#include <vector>
#include <memory>
#include <string>
#include "SecurityInterface.h"


class OuterMsgBuilderInterface
{
public:
    static const uint16_t magic = 0xABCD;
    static const uint8_t version = 0x01;

#pragma pack(push, 1)
    struct  Header
    {
        uint16_t magic;
        uint8_t version;
        uint32_t length;
        uint8_t flag;
    };
#pragma pack(pop)
    class UserMsg
    {
    public:
        ~UserMsg()
        {
            if (iv)
                delete[] iv;
            if (sha256)
                delete[] sha256;
        }
    public:
        std::unique_ptr<std::vector<uint8_t>> msg;
        uint8_t* iv;
        uint8_t* sha256;
    };
    enum class Flag : uint8_t {
        IS_BINARY = 1 << 0,
        IS_ENCRYPT = 1 << 1
    };

public:
    virtual ~OuterMsgBuilderInterface() {};
    virtual std::unique_ptr<UserMsg> buildMsg(std::string payload) = 0;
    virtual std::unique_ptr<UserMsg> buildMsg(std::vector<uint8_t> payload) = 0;
    virtual void setSecurityInstance(std::shared_ptr<SecurityInterface> instance) { security_instance = instance; }
private:
    virtual std::unique_ptr<UserMsg> build(std::vector<uint8_t> payload) = 0;
protected:
    std::shared_ptr<SecurityInterface> security_instance;
};


#endif //_MSGBUILDERINTERFACE_H