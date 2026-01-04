#ifndef _SIGNALJSONDRIVER_H
#define _SIGNALJSONDRIVER_H

#include "nlohmann/json.hpp"
#include "driver/interface/JsonFactoryInterface.h"

using json = nlohmann::json;

class SignalJsonMsgBuilder : public Json::JsonBuilder
{
public:
    std::string buildSignalMsg(Json::MessageType::Signal::Type type, std::map<std::string, std::string> args) override;
};

#endif