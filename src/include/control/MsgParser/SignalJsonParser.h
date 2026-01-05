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
    SignalJsonParser() {}
    void parse(const std::string &data) {}
    void setP2PInstace(std::shared_ptr<P2PInterface> inst)
    {
        p2p_instance = inst;
    }

private:
    std::unique_ptr<Json::JsonFactoryInterface> json_driver;
    std::map<std::string, std::function<void(std::unique_ptr<Json::Parser> parser)>> type_funcfion_map;
    std::shared_ptr<P2PInterface> p2p_instance;
};
#endif