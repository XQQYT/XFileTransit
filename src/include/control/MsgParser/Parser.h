#ifndef _PARSER_H
#define _PARSER_H

#include <memory>
#include "driver/interface/Network/NetworkInterface.h"
#include "control/MsgParser/UserJsonParser.h"
#include "control/MsgParser/SignalJsonParser.h"
#include "driver/interface/Network/P2PInterface.h"

class Parser
{
public:
    enum class MsgType
    {
        Signal,
        User
    };
    void parse(std::unique_ptr<NetworkInterface::UserMsg> data)
    {
        user_json_parser.parse(std::move(data));
    }
    void parse(const std::string &data, MsgType type)
    {
        switch (type)
        {
        case MsgType::Signal:
            signal_json_parser.parse(data);
            break;
        case MsgType::User:
            user_json_parser.parse(data);
            break;
        default:
            break;
        }
    }
    void setP2PInstance(std::shared_ptr<P2PInterface> inst)
    {
        signal_json_parser.setP2PInstace(inst);
    }
    void setWSInstance(std::shared_ptr<NetworkInterface> inst)
    {
        signal_json_parser.setWSInstace(inst);
    }
    void setOnP2PStatusChanged(std::function<void(P2PInterface::IceState)> cb)
    {
        signal_json_parser.setOnP2PStatusChanged(std::move(cb));
    }

private:
    UserJsonParser user_json_parser;
    SignalJsonParser signal_json_parser;
};

#endif