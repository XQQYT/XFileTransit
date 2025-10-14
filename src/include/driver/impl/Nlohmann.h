#ifndef _NLOHMANN_H
#define _NLOHMANN_H

#include "nlohmann/json.hpp"
#include "driver/interface/JsonFactoryInterface.h"

using json = nlohmann::json;

class NlohmannJsonParser : public Parser
{
public:
    NlohmannJsonParser() = default;
    explicit NlohmannJsonParser(const json& j) : msg_json(j) {}
    void loadJson(const std::string&) override;
    std::string getValue(const std::string& key) override;
    std::unique_ptr<Parser> getObj(const std::string& key) override;
    bool contain(const std::string& key) override;
    std::string toString() override;
    std::vector<std::unique_ptr<Parser>> getArray(const std::string& key) override;
private:
    json msg_json;
};

class NlohmannJson : public JsonFactoryInterface
{
public:
    std::unique_ptr<Parser> getParser() override;
    std::unique_ptr<JsonBuilder> getBuilder(const MsgType type) override;
};

class UserMsgBuilder : public JsonBuilder
{
public:
    std::string build(std::map<std::string, std::string>& args) override;
};
#endif