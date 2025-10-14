#include "driver/impl/Nlohmann.h"
#include <iostream>

void NlohmannJsonParser::loadJson(const std::string& msg)
{
    msg_json = json::parse(msg);
}
std::string NlohmannJsonParser::getValue(const std::string& key)
{
    if (msg_json.empty())
    {
        return  "";
    }
    if (msg_json[key].is_string())
    {
        return msg_json[key].get<std::string>();
    }

}
std::unique_ptr<Parser> NlohmannJsonParser::getObj(const std::string& key)
{
    if (msg_json.empty())
    {
        return nullptr;
    }
    if (msg_json[key].is_object())
    {
        return std::make_unique<NlohmannJsonParser>(msg_json[key]);
    }
    return nullptr;
}
bool NlohmannJsonParser::contain(const std::string& key)
{
    return msg_json.contains(key);
}
std::vector<std::unique_ptr<Parser>> NlohmannJsonParser::getArray(const std::string& key)
{
    if (msg_json.empty())
    {
        return {};
    }
    std::vector<std::unique_ptr<Parser>> result;

    if (msg_json.contains(key) && msg_json[key].is_array())
    {
        for (const auto& item : msg_json[key])
        {
            if (item.is_object())
            {
                result.push_back(std::make_unique<NlohmannJsonParser>(item));
            }
        }
    }

    return result;
}
std::string NlohmannJsonParser::toString()
{
    if (msg_json.empty())
    {
        return "";
    }
    return msg_json.dump();
}
std::unique_ptr<Parser> NlohmannJson::getParser()
{
    return std::make_unique<NlohmannJsonParser>();
}
std::unique_ptr<JsonBuilder> NlohmannJson::getBuilder(const MsgType type)
{
    switch (type)
    {
    case MsgType::User:
        return std::make_unique<UserMsgBuilder>();
    default:
        return nullptr;
    }
}


std::string UserMsgBuilder::build(std::map<std::string, std::string>& args)
{
    auto type = args["type"];
    json result_json;
    if (type == "connect_request")
    {
        result_json["type"] = type;
        result_json["content"] = {
            {"source_ip", args["source_ip"]},
            {"device_name", args["device_name"]}
        };
    }
    return result_json.dump();
}