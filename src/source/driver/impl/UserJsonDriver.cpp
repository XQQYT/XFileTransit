#include "driver/impl/UserJsonDriver.h"
#include "driver/impl/FileUtility.h"
#include <iostream>

void NlohmannJsonParser::loadJson(const std::string &msg)
{
    msg_json = json::parse(msg);
}
std::string NlohmannJsonParser::getValue(const std::string &&key)
{
    if (msg_json.empty())
    {
        return "";
    }
    if (msg_json[key].is_string())
    {
        return msg_json[key].get<std::string>();
    }
    return "";
}
std::optional<bool> NlohmannJsonParser::getBool(const std::string &&key)
{
    if (msg_json.empty() || !msg_json.contains(key))
    {
        return std::nullopt;
    }

    if (msg_json[key].is_boolean())
    {
        return msg_json[key].get<bool>();
    }

    return std::nullopt;
}
std::unique_ptr<Json::Parser> NlohmannJsonParser::getObj(const std::string &&key)
{
    if (msg_json.empty())
    {
        return nullptr;
    }
    if (msg_json[key].is_object())
    {
        return std::make_unique<NlohmannJsonParser>(msg_json[key]);
    }
    return nullptr;
}
bool NlohmannJsonParser::contain(const std::string &&key)
{
    return msg_json.contains(key);
}
std::vector<std::unique_ptr<Json::Parser>> NlohmannJsonParser::getArray(const std::string &&key)
{
    if (msg_json.empty())
    {
        return {};
    }
    std::vector<std::unique_ptr<Parser>> result;

    if (msg_json.contains(key) && msg_json[key].is_array())
    {
        for (const auto &item : msg_json[key])
        {
            result.push_back(std::make_unique<NlohmannJsonParser>(item));
        }
    }

    return result;
}
std::vector<std::string> NlohmannJsonParser::getArrayItems()
{
    if (msg_json.empty())
    {
        return {};
    }
    std::vector<std::string> items;
    for (const auto &item : msg_json)
    {
        if (item.is_string())
        {
            items.push_back(item);
        }
    }
    return items;
}
std::string NlohmannJsonParser::toString()
{
    if (msg_json.empty())
    {
        return "";
    }
    return msg_json.dump();
}
std::unique_ptr<Json::Parser> NlohmannJson::getParser()
{
    return std::make_unique<NlohmannJsonParser>();
}
std::unique_ptr<Json::JsonBuilder> NlohmannJson::getBuilder(const Json::BuilderType type)
{
    switch (type)
    {
    case Json::BuilderType::User:
        return std::make_unique<UserJsonMsgBuilder>();
    case Json::BuilderType::Sync:
        return std::make_unique<SyncJsonMsgBuilder>();
    case Json::BuilderType::File:
        return std::make_unique<FileJsonMsgBuilder>();
    case Json::BuilderType::Settings:
        return std::make_unique<SettingsJsonMsgBuilder>();
    default:
        return nullptr;
    }
}

std::string UserJsonMsgBuilder::buildUserMsg(Json::MessageType::User::Type type, std::map<std::string, std::string> &&args)
{
    if (!registry.validateFields(type, args))
    {
        throw std::invalid_argument("Missing required fields for message type");
    }

    const auto &schema = registry.getSchema(type);

    json result_json;
    json content_json;

    result_json["type"] = schema.type_name;

    for (auto &&[key, value] : args)
    {
        content_json[std::forward<decltype(key)>(key)] = std::forward<decltype(value)>(value);
    }

    result_json["content"] = std::move(content_json);

    return result_json.dump();
}

std::string SyncJsonMsgBuilder::buildSyncMsg(Json::MessageType::Sync::Type type, std::vector<std::string> &&args, uint8_t stride)
{
    json result_json;
    result_json["type"] = Json::MessageType::Sync::toString(type);

    json content_json = json::object();

    if (type == Json::MessageType::Sync::Type::RemoveFile)
    {
        // removeFiles 使用简单数组结构: {"files":["2","3","5"]}
        content_json["files"] = args;
    }
    else
    {
        // 其他类型使用原来的嵌套数组结构
        json files_array = json::array();

        // 按步长处理参数
        for (size_t i = 0; i < args.size(); i += stride)
        {
            json group = json::array();
            for (size_t j = 0; j < stride && (i + j) < args.size(); ++j)
            {
                group.push_back(args[i + j]);
            }
            files_array.push_back(group);
        }

        content_json["files"] = files_array;
    }

    result_json["content"] = content_json;
    return result_json.dump();
}

void FileJsonMsgBuilder::buildFileHeader(json &result, Json::MessageType::File::Type type, const std::map<std::string, std::string> &args)
{
    try
    {
        json content;
        result["type"] = Json::MessageType::File::toString(type);

        content["id"] = args.at("id");
        content["total_size"] = args.at("total_size");
        content["total_blocks"] = args.at("total_blocks");

        result["content"] = content;
    }
    catch (const std::out_of_range &e)
    {
        throw std::runtime_error("Missing required field in file header");
    }
}

void FileJsonMsgBuilder::buildDirHeader(json &result, Json::MessageType::File::Type type, const std::map<std::string, std::string> &args)
{
    try
    {
        json content;
        result["type"] = Json::MessageType::File::toString(type);
        content["id"] = args.at("id");
        content["leaf_paths"] = json::parse(args.at("leaf_paths"));
        content["total_paths"] = args.at("total_paths");
        content["total_size"] = args.at("total_size");

        result["content"] = content;
    }
    catch (const std::out_of_range &e)
    {
        throw std::runtime_error("Missing required field in file header");
    }
}

void FileJsonMsgBuilder::buildDirItemHeader(json &result, Json::MessageType::File::Type type, const std::map<std::string, std::string> &args)
{
    try
    {
        json content;
        result["type"] = Json::MessageType::File::toString(type);

        content["id"] = args.at("id");
        content["path"] = args.at("path");
        content["total_size"] = args.at("total_size");
        content["total_blocks"] = args.at("total_blocks");

        result["content"] = content;
    }
    catch (const std::out_of_range &e)
    {
        throw std::runtime_error("Missing required field in file header");
    }
}

std::string FileJsonMsgBuilder::buildFileMsg(Json::MessageType::File::Type type, std::map<std::string, std::string> args)
{
    json result;
    json content;
    switch (type)
    {
    case Json::MessageType::File::Type::FileHeader:
        buildFileHeader(result, type, args);
        break;
    case Json::MessageType::File::DirectoryHeader:
        buildDirHeader(result, type, args);
        break;
    case Json::MessageType::File::DirectoryItemHeader:
        buildDirItemHeader(result, type, args);
        break;
    case Json::MessageType::File::FileEnd:
        result["type"] = Json::MessageType::File::toString(Json::MessageType::File::FileEnd);
        result["content"] = content;
        return result.dump();
    case Json::MessageType::File::FileCanceled:
        result["type"] = Json::MessageType::File::toString(Json::MessageType::File::FileCanceled);
        result["content"] = content;
        return result.dump();
    case Json::MessageType::File::FileCancel:
        result["type"] = Json::MessageType::File::toString(Json::MessageType::File::FileCancel);
        content["id"] = args["id"];
        result["content"] = content;
        return result.dump();
    case Json::MessageType::File::ReceiverInitDone:
        result["type"] = Json::MessageType::File::toString(Json::MessageType::File::ReceiverInitDone);
        result["content"] = content;
        return result.dump();
    default:
        break;
    }
    return result.dump();
}

std::string SettingsJsonMsgBuilder::buildSettingsMsg(Json::MessageType::Settings::Type type, std::map<std::string, std::string> args)
{
    json result_json;
    json content_json;

    result_json["type"] = Json::MessageType::Settings::toString(type);

    switch (type)
    {
    case Json::MessageType::Settings::ConcurrentTask:
        content_json["concurrent"] = args["concurrent"];
        break;
    default:
        break;
    }

    result_json["content"] = std::move(content_json);

    return result_json.dump();
}
