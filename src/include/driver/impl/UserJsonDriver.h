#ifndef _USERJSONDRIVER_H
#define _USERJSONDRIVER_H

#include "nlohmann/json.hpp"
#include "driver/interface/JsonFactoryInterface.h"

using json = nlohmann::json;

class NlohmannJsonParser : public Json::Parser
{
public:
    NlohmannJsonParser() = default;
    explicit NlohmannJsonParser(const json &j) : msg_json(j) {}
    void loadJson(const std::string &) override;
    std::string getValue(const std::string &&key) override;
    std::optional<bool> getBool(const std::string &&key) override;
    std::unique_ptr<Parser> getObj(const std::string &&key) override;
    bool contain(const std::string &&key) override;
    std::string toString() override;
    std::vector<std::unique_ptr<Parser>> getArray(const std::string &&key) override;
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

class UserJsonMsgBuilder : public Json::JsonBuilder
{
public:
    std::string buildUserMsg(Json::MessageType::User::Type type, std::map<std::string, std::string> &&args) override;

private:
    Json::MessageRegistry registry;
};

class SyncJsonMsgBuilder : public Json::JsonBuilder
{
public:
    std::string buildSyncMsg(Json::MessageType::Sync::Type type, std::vector<std::string> &&args, uint8_t stride) override;
};

class FileJsonMsgBuilder : public Json::JsonBuilder
{
public:
    static std::string vectorToJsonString(const std::vector<std::string> &array)
    {
        nlohmann::json jArray = array;
        return jArray.dump();
    }
    std::string buildFileMsg(Json::MessageType::File::Type type, std::map<std::string, std::string> args) override;

private:
    void buildFileHeader(json &result, Json::MessageType::File::Type type, const std::map<std::string, std::string> &args);
    void buildDirHeader(json &result, Json::MessageType::File::Type type, const std::map<std::string, std::string> &args);
    void buildDirItemHeader(json &result, Json::MessageType::File::Type type, const std::map<std::string, std::string> &args);
};

class SettingsJsonMsgBuilder : public Json::JsonBuilder
{
public:
    std::string buildSettingsMsg(Json::MessageType::Settings::Type type, std::map<std::string, std::string> args) override;
};
#endif