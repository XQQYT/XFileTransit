#ifndef _NLOHMANN_H
#define _NLOHMANN_H

#include "nlohmann/json.hpp"
#include "driver/interface/JsonFactoryInterface.h"

using json = nlohmann::json;

class NlohmannJsonParser : public Json::Parser
{
public:
    NlohmannJsonParser() = default;
    explicit NlohmannJsonParser(const json& j) : msg_json(j) {}
    void loadJson(const std::string&) override;
    std::string getValue(const std::string&& key) override;
    std::unique_ptr<Parser> getObj(const std::string&& key) override;
    bool contain(const std::string&& key) override;
    std::string toString() override;
    std::vector<std::unique_ptr<Parser>> getArray(const std::string&& key) override;
private:
    json msg_json;
};

class NlohmannJson : public Json::JsonFactoryInterface
{
public:
    std::unique_ptr<Json::Parser> getParser() override;
    std::unique_ptr<Json::JsonBuilder> getBuilder(const Json::BuilderType type) override;
};

class UserMsgBuilder : public Json::JsonBuilder
{
public:
    template<class Map>
    std::string buildImpl(uint64_t type, Map&& args);
    std::string build(uint64_t type, std::map<std::string, std::string>& args) override
    {
        return buildImpl(type, args);
    }
    std::string build(uint64_t type, std::map<std::string, std::string>&& args) override
    {
        return buildImpl(type, std::move(args));
    }
private:
    uint64_t current_type;
    std::map<std::string, std::string> fields;
    Json::MessageRegistry registry;
};
#endif