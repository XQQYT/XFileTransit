#include "control/MsgParser/JsonParser.h"
#include "driver/impl/Nlohmann.h"
#include <control/EventBusManager.h>
#include <iostream>

JsonParser::JsonParser() :
    json_driver(std::make_unique<NlohmannJson>())
{
    type_funcfion_map["connect_request"] = std::bind(JsonParser::connectRequest, this, std::placeholders::_1);
    type_funcfion_map["response"] = std::bind(JsonParser::resonpeResult, this, std::placeholders::_1);
}

void JsonParser::parse(std::vector<uint8_t>&& data)
{
    auto parser = json_driver->getParser();
    std::string data_str(std::make_move_iterator(data.begin()),
        std::make_move_iterator(data.end()));
    parser->loadJson(data_str);
    std::string type = parser->getValue("type");
    auto deal_func = type_funcfion_map.find(type);
    if (deal_func != type_funcfion_map.end())
    {
        auto contend_obj = parser->getObj("content");
        deal_func->second(std::move(contend_obj));
    }
    else
    {
        return;
    }
}

void JsonParser::connectRequest(std::unique_ptr<Json::Parser> parser)
{
    EventBusManager::instance().publish("/network/have_connect_request",
        parser->getValue("sender_device_ip"),
        parser->getValue("sender_device_name"));
}

void JsonParser::resonpeResult(std::unique_ptr<Json::Parser> parser)
{
    auto arg0 = JsonMessageType::parseResultType(parser->getValue("arg0"));
    auto subtype = JsonMessageType::parseResponseType(parser->getValue("subtype"));
    switch(subtype)
    {
        case JsonMessageType::ResponseType::CONNECT_REQUEST_RESPONSE:
            publishResponse("/network/have_connect_request_result", arg0);
            break;
        default:
            break;
    }
}

void JsonParser::publishResponse(std::string&& event_name, JsonMessageType::ResultType type)
{
    switch(type)
    {
        case JsonMessageType::ResultType::SUCCESS:
            EventBusManager::instance().publish(event_name, true);
            break;
        case JsonMessageType::ResultType::FAILED:
            EventBusManager::instance().publish(event_name, false);
            break;
        case JsonMessageType::ResultType::UNKNOWN:
            break;
        default:
            break;
    }
}
