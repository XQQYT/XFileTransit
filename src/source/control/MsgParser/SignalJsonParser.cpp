#include "control/MsgParser/SignalJsonParser.h"
#include "driver/impl/UserJsonDriver.h"
#include "driver/impl/SignalJsonDriver.h"

SignalJsonParser::SignalJsonParser() : json_driver(std::make_unique<NlohmannJson>()),
                                       signal_json_builder(std::make_unique<SignalJsonMsgBuilder>())
{
    type_funcfion_map["register_result"] = std::bind(&SignalJsonParser::onRegisterResult, this, std::placeholders::_1);
}

void SignalJsonParser::parse(const std::string &data_str)
{
    auto parser = json_driver->getParser();
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

void SignalJsonParser::onRegisterResult(std::unique_ptr<Json::Parser> parser)
{
    p2p_instance->initialize();
    bool ret = parser->getValue("status") == "success";
    if (ret)
    {
        p2p_instance->createOffer([this](const std::string &str)
                                  { p2p_instance->sendMsg(
                                        signal_json_builder->buildSignalMsg(Json::MessageType::Signal::Offer, {{"offer", str}})); });
    }
}
