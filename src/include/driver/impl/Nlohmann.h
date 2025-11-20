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
    std::optional<bool> getBool(const std::string&& key) override;
    std::unique_ptr<Parser> getObj(const std::string&& key) override;
    bool contain(const std::string&& key) override;
    std::string toString() override;
    std::vector<std::unique_ptr<Parser>> getArray(const std::string&& key) override;
    std::vector<std::string> getArrayItems() override;
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
    std::string buildUserMsg(Json::MessageType::User::Type type, std::map<std::string, std::string>&& args) override;
    std::string buildSyncMsg(Json::MessageType::Sync::Type type, std::vector<std::string>&& args, uint8_t stride) override
    {
        return "Don't use UserMsgBuilder to build SyncMsg";
    }
private:
    Json::MessageRegistry registry;
};

class SyncMsgBuilder : public Json::JsonBuilder
{
public:
    std::string buildUserMsg(Json::MessageType::User::Type type, std::map<std::string, std::string>&& args) override
    {
        return "Don't use SyncMsgBuilder to build UserMsg";
    }
    std::string buildSyncMsg(Json::MessageType::Sync::Type type, std::vector<std::string>&& args, uint8_t stride) override;
};
#endif