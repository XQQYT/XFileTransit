#include "control/MsgParser/SignalJsonParser.h"
#include "driver/impl/UserJsonDriver.h"
#include "driver/impl/SignalJsonDriver.h"
#include "control/EventBusManager.h"
#include "control/GlobalStatusManager.h"

SignalJsonParser::SignalJsonParser() : json_driver(std::make_unique<NlohmannJson>()),
                                       signal_json_builder(std::make_unique<SignalJsonMsgBuilder>())
{
    type_funcfion_map["register_result"] = std::bind(&SignalJsonParser::onRegisterResult, this, std::placeholders::_1);
    type_funcfion_map["target_status"] = std::bind(&SignalJsonParser::onTargetStatus, this, std::placeholders::_1);
    type_funcfion_map["connect_request"] = std::bind(&SignalJsonParser::onConnectRequest, this, std::placeholders::_1);
    type_funcfion_map["connect_request_result"] = std::bind(&SignalJsonParser::onConnectRequestResult, this, std::placeholders::_1);
    type_funcfion_map["sdp_offer"] = std::bind(&SignalJsonParser::onSdpOffer, this, std::placeholders::_1);
    type_funcfion_map["sdp_answer"] = std::bind(&SignalJsonParser::onSdpAnswer, this, std::placeholders::_1);
    type_funcfion_map["ice_candidate"] = std::bind(&SignalJsonParser::onIceCandidate, this, std::placeholders::_1);
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

    bool ret = parser->getValue("status") == "success";
    if (ret)
    {
        p2p_instance->initialize();
        p2p_instance->setIceGenerateCb(std::bind(&SignalJsonParser::onIceGenerated, this, std::placeholders::_1));
        p2p_instance->setIceStatusCb(std::bind(&SignalJsonParser::onIceStatusChanged, this, std::placeholders::_1));
    }
}

void SignalJsonParser::onTargetStatus(std::unique_ptr<Json::Parser> parser)
{
    bool online = parser->getValue("status") == "true" ? true : false;
    if (parser->getValue("target_code") != TargetInfo::target_code)
    {
        return;
    }

    if (online)
    {
        TargetInfo::target_status = TargetStatus::Online;
        std::unordered_map<std::string, std::string>
            args;
        args["target_code"] = TargetInfo::target_code;
        args["password"] = TargetInfo::target_password;
        EventBusManager::instance().publish("/network/send_connect_request", args);
    }
    else
    {
        TargetInfo::target_status = TargetStatus::Offline;
        EventBusManager::instance().publish("/signal/target_status", online);
    }
}
void SignalJsonParser::onConnectRequest(std::unique_ptr<Json::Parser> parser)
{
    ConnectionInfo::connection_type = ConnectionType::P2P;
    std::string sender_code = parser->getValue("sender_code");
    std::string password = parser->getValue("password");
    EventBusManager::instance().publish("/network/have_connect_request", sender_code, std::string("Remote"));
    TargetInfo::target_code = sender_code;
}

void SignalJsonParser::onConnectRequestResult(std::unique_ptr<Json::Parser> parser)
{
    EventBusManager::instance().publish("/network/have_connect_request_result", parser->getValue("result") == "true" ? true : false, TargetInfo::target_code);
}

void SignalJsonParser::onSdpOffer(std::unique_ptr<Json::Parser> parser)
{
    p2p_instance->setRemoteDescription(parser->getValue("offer"), [&](bool ret, const std::string &answer)
                                       { 
                                        if(ret)
                                        {
                                                p2p_instance->sendMsg(signal_json_builder->buildSignalMsg(
                                                    Json::MessageType::Signal::Answer,
                                                    {{"sender_code", ConnectionInfo::my_code},
                                                    {"target_code", TargetInfo::target_code},
                                                    {"answer", answer}}));
                                                } });
}

void SignalJsonParser::onSdpAnswer(std::unique_ptr<Json::Parser> parser)
{
    p2p_instance->setRemoteDescription(parser->getValue("answer"), nullptr);
}

void SignalJsonParser::onIceGenerated(const std::string &ice)
{
    p2p_instance->sendMsg(signal_json_builder->buildSignalMsg(
        Json::MessageType::Signal::IceCandidate,
        {{"sender_code", ConnectionInfo::my_code},
         {"target_code", TargetInfo::target_code},
         {"candidate", ice}}));
}

void SignalJsonParser::onIceCandidate(std::unique_ptr<Json::Parser> parser)
{
    p2p_instance->addIceCandidate(parser->getValue("candidate"));
}

void SignalJsonParser::onIceStatusChanged(const P2PInterface::IceState state)
{
    switch (state)
    {
    case P2PInterface::IceState::Completed:
        EventBusManager::instance().publish("/network/have_connect_request_result", true, TargetInfo::target_code);
        break;
    case P2PInterface::IceState::Failed:
        EventBusManager::instance().publish("/network/have_connect_request_result", false, TargetInfo::target_code);
        break;
    case P2PInterface::IceState::Closed:
    case P2PInterface::IceState::Disconnected:
    default:
        break;
    }
}