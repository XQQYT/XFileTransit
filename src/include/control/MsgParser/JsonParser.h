#ifndef _JSONPARSER_H
#define _JSONPARSER_H

#include "Parser.h"
#include "driver/interface/JsonFactoryInterface.h"
#include <map>
#include <functional>
#include <string>

namespace JsonMessageType {
    enum class ResponseType {
        CONNECT_REQUEST_RESPONSE,
        UNKNOWN
    };

    enum class ResultType {
        SUCCESS,
        FAILED,
        UNKNOWN
    };

    inline ResponseType parseResponseType(const std::string& type) {
        static const std::map<std::string, ResponseType> mapping = {
            {"connect_request_response", ResponseType::CONNECT_REQUEST_RESPONSE},
        };

        auto it = mapping.find(type);
        return it != mapping.end() ? it->second : ResponseType::UNKNOWN;
    }

    inline ResultType parseResultType(const std::string& result) {
        static const std::map<std::string, ResultType> mapping = {
            {"success", ResultType::SUCCESS},
            {"failed", ResultType::FAILED}
        };

        auto it = mapping.find(result);
        return it != mapping.end() ? it->second : ResultType::UNKNOWN;
    }
}

class JsonParser : public Parser
{
public:
    JsonParser();
    void parse(std::unique_ptr<NetworkInterface::UserMsg> data) override;
private:
    void connectRequest(std::unique_ptr<Json::Parser> parser);
    void cancelConnRequest(std::unique_ptr<Json::Parser> parser);
    void resonpeResult(std::unique_ptr<Json::Parser> parser);
    void syncAddFiles(std::unique_ptr<Json::Parser> parser);
    void syncDeleteFiles(std::unique_ptr<Json::Parser> parser);
    void downloadFile(std::unique_ptr<Json::Parser> parser);
    void publishResponse(std::string&& event_name, JsonMessageType::ResultType type);
    void publishResponse(std::string&& event_name, JsonMessageType::ResultType type, std::string arg0);
private:
    std::unique_ptr <Json::JsonFactoryInterface> json_driver;
    std::map<std::string, std::function<void(std::unique_ptr<Json::Parser> parser)>> type_funcfion_map;
};
#endif