#include "driver/impl/SignalJsonDriver.h"

std::string SignalJsonMsgBuilder::buildSignalMsg(Json::MessageType::Signal::Type type, std::map<std::string, std::string> args)
{
    json result;
    json content;
    result["type"] = Json::MessageType::Signal::toString(type);

    switch (type)
    {
    case Json::MessageType::Signal::Register:
        content["id"] = args["id"];
        break;
    case Json::MessageType::Signal::Offer:
        content["offer"] = args["offer"];
        content["target_code"] = args["target_code"];
        content["sender_code"] = args["sender_code"];
        break;
    case Json::MessageType::Signal::ConnectRequest:
        content["target_code"] = args["target_code"];
        content["password"] = args["password"];
        content["sender_code"] = args["sender_code"];
        break;
    case Json::MessageType::Signal::GetTargetStatus:
        content["target_code"] = args["target_code"];
        content["sender_code"] = args["sender_code"];
        break;
    case Json::MessageType::Signal::ConnectRequestResult:
        content["target_code"] = args["target_code"];
        content["sender_code"] = args["sender_code"];
        content["result"] = args["result"];
        break;
    default:
        break;
    }

    result["content"] = content;
    return result.dump();
}