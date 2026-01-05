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
    void parse(std::unique_ptr<NetworkInterface::UserMsg> data)
    {
        user_json_parser.parse(std::move(data));
    }
    void parse(const std::string &data)
    {
        signal_json_parser.parse(data);
    }
    void setP2PInstance(std::shared_ptr<P2PInterface> inst)
    {
        signal_json_parser.setP2PInstace(inst);
    }

private:
    UserJsonParser user_json_parser;
    SignalJsonParser signal_json_parser;
};

#endif