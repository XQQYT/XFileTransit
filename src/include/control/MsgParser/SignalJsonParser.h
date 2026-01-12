#ifndef _SIGNALJSONPARSER_H
#define _SIGNALJSONPARSER_H

#include "driver/interface/JsonFactoryInterface.h"
#include "driver/interface/Network/P2PInterface.h"
#include <map>
#include <functional>
#include <string>

class SignalJsonParser
{
public:
    SignalJsonParser();
    void parse(const std::string &data);
    inline void setP2PInstace(std::shared_ptr<P2PInterface> inst)
    {
        p2p_instance = inst;
        p2p_instance->setIceGenerateCb(std::bind(&SignalJsonParser::onIceGenerated, this, std::placeholders::_1));
    }
    inline void setWSInstace(std::shared_ptr<NetworkInterface> inst)
    {
        websocket_instance = inst;
    }

private:
    void onRegisterResult(std::unique_ptr<Json::Parser> parser);
    void onTargetStatus(std::unique_ptr<Json::Parser> parser);
    void onConnectRequest(std::unique_ptr<Json::Parser> parser);
    void onConnectRequestResult(std::unique_ptr<Json::Parser> parser);
    void onSdpOffer(std::unique_ptr<Json::Parser> parser);
    void onSdpAnswer(std::unique_ptr<Json::Parser> parser);
    void onIceCandidate(std::unique_ptr<Json::Parser> parser);

private:
    void onIceGenerated(const std::string &ice);

private:
    std::unique_ptr<Json::JsonFactoryInterface> json_driver;
    std::unique_ptr<Json::JsonBuilder> signal_json_builder;
    std::map<std::string, std::function<void(std::unique_ptr<Json::Parser> parser)>> type_funcfion_map;
    std::shared_ptr<P2PInterface> p2p_instance;
    std::shared_ptr<NetworkInterface> websocket_instance;
};
#endif