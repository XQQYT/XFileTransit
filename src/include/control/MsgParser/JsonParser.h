#ifndef _JSONPARSER_H
#define _JSONPARSER_H

#include "Parser.h"
#include "driver/interface/JsonFactoryInterface.h"
#include <unordered_map>
#include <functional>
#include <string>

class JsonParser : public Parser
{
public:
    JsonParser();
    void parse(std::vector<uint8_t>&& data) override;
private:
    void connectRequest(std::unique_ptr<Json::Parser> parser);
private:
    std::unique_ptr <Json::JsonFactoryInterface> json_driver;
    std::unordered_map<std::string, std::function<void(std::unique_ptr<Json::Parser> parser)>> type_funcfion_map;
};
#endif