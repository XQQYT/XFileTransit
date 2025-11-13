#include "control/MsgParser/JsonParser.h"
#include "driver/impl/Nlohmann.h"
#include <control/EventBusManager.h>
#include <iostream>

JsonParser::JsonParser() :
    json_driver(std::make_unique<NlohmannJson>())
{
    type_funcfion_map["connect_request"] = std::bind(JsonParser::connectRequest, this, std::placeholders::_1);
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
    std::cout << "have connect request" << std::endl;
}