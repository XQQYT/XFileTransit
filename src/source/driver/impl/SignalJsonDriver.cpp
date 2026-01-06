#include "driver/impl/SignalJsonDriver.h"

std::string SignalJsonMsgBuilder::buildSignalMsg(Json::MessageType::Signal::Type type, std::map<std::string, std::string> args)
{
    json result;
    json content;
    result["type"] = Json::MessageType::Signal::toString(type);

    switch (type)
    {
    case Json::MessageType::Signal::Register:
        content["id"] = "123456";

        break;
    case Json::MessageType::Signal::Offer:
        content["offer"] = args["offer"];
        break;
    case Json::MessageType::Signal::ConnectRequest:
        content["code"] = args["code"];
        content["password"] = args["password"];
        content["sender_code"] = args["sender_code"];
    default:
        break;
    }

    result["content"] = content;
    return result.dump();
}