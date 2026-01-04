#include "driver/impl/SignalJsonDriver.h"

std::string SignalJsonMsgBuilder::buildSignalMsg(Json::MessageType::Signal::Type type, std::map<std::string, std::string> args)
{
    json result;
    json content;
    switch (type)
    {
    case Json::MessageType::Signal::Register:
        result["type"] = Json::MessageType::Signal::toString(type);
        content["id"] = "123456";
        result["content"] = content;
        return result.dump();
    default:
        return "";
    }
}