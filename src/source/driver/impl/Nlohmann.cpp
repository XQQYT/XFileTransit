#include "driver/impl/Nlohmann.h"
#include <iostream>

void NlohmannJsonParser::loadJson(const std::string& msg)
{
    msg_json = json::parse(msg);
}
std::string NlohmannJsonParser::getValue(const std::string&& key)
{
    if (msg_json.empty())
    {
        return  "";
    }
    if (msg_json[key].is_string())
    {
        return msg_json[key].get<std::string>();
    }
    return "";
}
std::optional<bool> NlohmannJsonParser::getBool(const std::string&& key) 
{
    if (msg_json.empty() || !msg_json.contains(key)) {
        return std::nullopt;
    }
    
    if (msg_json[key].is_boolean()) {
        return msg_json[key].get<bool>();
    }
    
    return std::nullopt;
}
std::unique_ptr<Json::Parser> NlohmannJsonParser::getObj(const std::string&& key)
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
bool NlohmannJsonParser::contain(const std::string&& key)
{
    return msg_json.contains(key);
}
std::vector<std::unique_ptr<Json::Parser>> NlohmannJsonParser::getArray(const std::string&& key)
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
std::unique_ptr<Json::Parser> NlohmannJson::getParser()
{
    return std::make_unique<NlohmannJsonParser>();
}
std::unique_ptr<Json::JsonBuilder> NlohmannJson::getBuilder(const Json::BuilderType type)
{
    switch (type)
    {
    case Json::BuilderType::User:
        return std::make_unique<UserMsgBuilder>();
    default:
        return nullptr;
    }
}

template<class Map>
std::string UserMsgBuilder::buildImpl(uint64_t type, Map&& args)
{
    if (!registry.validateFields(type, args)) {
        throw std::invalid_argument("Missing required fields for message type");
    }

    const auto& schema = registry.getSchema(type);

    json result_json;
    json content_json;

    result_json["type"] = schema.type_name;

    for (auto&& [key, value] : args) {
        content_json[std::forward<decltype(key)>(key)] = std::forward<decltype(value)>(value);
    }

    result_json["content"] = std::move(content_json);

    return result_json.dump();
}