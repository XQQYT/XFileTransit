#include "control/MsgParser/UserJsonParser.h"
#include "driver/impl/UserJsonDriver.h"
#include "control/EventBusManager.h"
#include "control/GlobalStatusManager.h"
#include <iostream>

UserJsonParser::UserJsonParser() : json_driver(std::make_unique<NlohmannJson>())
{
    type_funcfion_map["connect_request"] = std::bind(&UserJsonParser::connectRequest, this, std::placeholders::_1);
    type_funcfion_map["response"] = std::bind(&UserJsonParser::resonpeResult, this, std::placeholders::_1);
    type_funcfion_map["cancel_conn_request"] = std::bind(&UserJsonParser::cancelConnRequest, this, std::placeholders::_1);

    type_funcfion_map[Json::MessageType::Sync::toString(Json::MessageType::Sync::FileExpired)] = std::bind(&UserJsonParser::syncExpiredFile, this, std::placeholders::_1);
    type_funcfion_map[Json::MessageType::Sync::toString(Json::MessageType::Sync::AddFiles)] = std::bind(&UserJsonParser::syncAddFiles, this, std::placeholders::_1);
    type_funcfion_map[Json::MessageType::Sync::toString(Json::MessageType::Sync::RemoveFile)] = std::bind(&UserJsonParser::syncDeleteFiles, this, std::placeholders::_1);
    type_funcfion_map[Json::MessageType::Sync::toString(Json::MessageType::Sync::DownloadFile)] = std::bind(&UserJsonParser::downloadFile, this, std::placeholders::_1);

    type_funcfion_map[Json::MessageType::Settings::toString(Json::MessageType::Settings::ConcurrentTask)] = std::bind(&UserJsonParser::concurrentChanged, this, std::placeholders::_1);
    type_funcfion_map[Json::MessageType::File::toString(Json::MessageType::File::FileCancel)] = std::bind(&UserJsonParser::cancelFileTransit, this, std::placeholders::_1);
    type_funcfion_map[Json::MessageType::File::toString(Json::MessageType::File::ReceiverInitDone)] = std::bind(&UserJsonParser::receiverInitDone, this, std::placeholders::_1);
}

void UserJsonParser::parse(std::unique_ptr<NetworkInterface::UserMsg> data)
{
    auto parser = json_driver->getParser();
    std::string data_str(std::make_move_iterator(data->data.begin()),
                         std::make_move_iterator(data->data.end()));
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

void UserJsonParser::connectRequest(std::unique_ptr<Json::Parser> parser)
{
    std::string ip = parser->getValue("sender_device_ip");
    std::string name = parser->getValue("sender_device_name");
    EventBusManager::instance().publish("/network/have_connect_request", parser->getValue("sender_device_ip"), parser->getValue("sender_device_name"));
    GlobalStatusManager::getInstance().setCurrentTargetDeviceIP(std::move(ip));
    GlobalStatusManager::getInstance().setCurrentTargetDeviceName(std::move(name));
}

void UserJsonParser::resonpeResult(std::unique_ptr<Json::Parser> parser)
{
    auto arg0 = JsonMessageType::parseResultType(parser->getValue("arg0"));
    auto subtype = JsonMessageType::parseResponseType(parser->getValue("subtype"));
    switch (subtype)
    {
    case JsonMessageType::ResponseType::CONNECT_REQUEST_RESPONSE:
        publishResponse("/network/have_connect_request_result", arg0,
                        GlobalStatusManager::getInstance().getCurrentTargetDeviceIP());
        break;
    default:
        break;
    }
}

void UserJsonParser::publishResponse(std::string &&event_name, JsonMessageType::ResultType type)
{
    switch (type)
    {
    case JsonMessageType::ResultType::SUCCESS:
        EventBusManager::instance().publish(event_name, true);
        break;
    case JsonMessageType::ResultType::FAILED:
        EventBusManager::instance().publish(event_name, false);
        break;
    case JsonMessageType::ResultType::UNKNOWN:
        break;
    default:
        break;
    }
}

void UserJsonParser::publishResponse(std::string &&event_name, JsonMessageType::ResultType type, std::string arg0)
{
    switch (type)
    {
    case JsonMessageType::ResultType::SUCCESS:
        EventBusManager::instance().publish(event_name, true, arg0);
        break;
    case JsonMessageType::ResultType::FAILED:
        EventBusManager::instance().publish(event_name, false, arg0);
        break;
    case JsonMessageType::ResultType::UNKNOWN:
        break;
    default:
        break;
    }
}

void UserJsonParser::cancelConnRequest(std::unique_ptr<Json::Parser> parser)
{
    std::string ip = parser->getValue("sender_device_ip");
    std::string name = parser->getValue("sender_device_name");
    EventBusManager::instance().publish("/network/cancel_conn_request", parser->getValue("sender_device_ip"), parser->getValue("sender_device_name"));
}

void UserJsonParser::syncExpiredFile(std::unique_ptr<Json::Parser> parser)
{
    std::vector<std::string> files;
    auto file_ids = parser->getArray("files");
    for (const auto &id : file_ids)
    {
        auto tmp = id->getArrayItems();
        files.insert(files.end(), tmp.begin(), tmp.end());
    }
    EventBusManager::instance().publish("/sync/have_expired_file", files);
}
void UserJsonParser::syncAddFiles(std::unique_ptr<Json::Parser> parser)
{
    auto array = parser->getArray("files");
    std::vector<std::vector<std::string>> files;
    for (auto &cur_arr : array)
    {
        files.push_back(cur_arr->getArrayItems());
    }
    EventBusManager::instance().publish("/sync/have_addfiles", files);
}

void UserJsonParser::syncDeleteFiles(std::unique_ptr<Json::Parser> parser)
{
    std::vector<std::string> files;
    auto file_ids = parser->getArray("files");
    for (const auto &id : file_ids)
    {
        auto tmp = id->getArrayItems();
        files.insert(files.end(), tmp.begin(), tmp.end());
    }
    EventBusManager::instance().publish("/sync/have_deletefiles", files);
}

void UserJsonParser::downloadFile(std::unique_ptr<Json::Parser> parser)
{
    std::vector<std::string> files;
    auto file_ids = parser->getArray("files");
    for (const auto &id : file_ids)
    {
        auto tmp = id->getArrayItems();
        files.insert(files.end(), tmp.begin(), tmp.end());
    }
    EventBusManager::instance().publish("/file/have_download_request", files);
}

void UserJsonParser::concurrentChanged(std::unique_ptr<Json::Parser> parser)
{
    uint8_t concurrent_num = std::stoi(parser->getValue("concurrent"));
    EventBusManager::instance().publish("/settings/have_concurrent_changed", concurrent_num);
}

void UserJsonParser::cancelFileTransit(std::unique_ptr<Json::Parser> parser)
{
    EventBusManager::instance().publish("/file/have_cancel_transit", static_cast<uint32_t>(std::stoul(parser->getValue("id"))));
}

void UserJsonParser::receiverInitDone(std::unique_ptr<Json::Parser> parser)
{
    EventBusManager::instance().publish("/file/have_init_file_receiver_done");
}