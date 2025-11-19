#ifndef _JSONFACTORY_H
#define _JSONFACTORY_H

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <optional>

namespace Json
{
    //构建器类型定义
    enum class BuilderType
    {
        User,
        File,
        Sync
    };
    // 消息类型定义
    namespace MessageType
    {
        namespace User
        {
            enum Type
            {
                ConnectRequest,
                ConnectRequestResponse
            };

            constexpr const char* toString(Type type)
            {
                switch (type)
                {
                case ConnectRequest: return "ConnectRequest";
                case ConnectRequestResponse: return "ConnectRequestResponse";
                default: return "unknown";
                }
            }
        }

        namespace File
        {
            // enum Type
            // {

            // };

            // constexpr const char* toString(Type type)
            // {

            // }
        }

        namespace Sync
        {
            enum Type
            {
                AddFiles,
                RemoveFile
            };

            constexpr const char* toString(Type type)
            {
                switch (type)
                {
                case AddFiles: return "add_files";
                case RemoveFile: return "remove_files";
                default: return "unknown";
                }
            }
        }
    }

    // 消息架构描述
    struct MessageSchema
    {
        uint64_t type;
        std::string type_name;
        std::vector<std::string> required_fields;
    };

    // 消息注册表
    class MessageRegistry
    {
    private:
        std::map<int, MessageSchema> schemas;

    public:
        MessageRegistry()
        {
            registerSchema(MessageType::User::ConnectRequest, "connect_request",
                { "sender_device_name", "sender_device_ip" });
            registerSchema(MessageType::User::ConnectRequestResponse, "response",
                { "subtype", "arg0" });
        }

        void registerSchema(uint64_t type, const std::string& type_name,
            const std::vector<std::string>& required)
        {
            schemas[type] = { type, type_name, required };
        }

        const MessageSchema& getSchema(uint64_t type) const {
            return schemas.at(type);
        }

        bool validateFields(uint64_t type, const std::map<std::string, std::string>& fields) const
        {
            const auto& schema = getSchema(type);
            for (const auto& required : schema.required_fields)
            {
                if (!fields.count(required))
                    return false;
            }
            return true;
        }
    };
    class Parser
    {
    public:
        virtual void loadJson(const std::string& content) = 0;
        virtual std::string getValue(const std::string&& key) = 0;
        virtual std::optional<bool> getBool(const std::string&& key) = 0;
        virtual std::unique_ptr<Parser> getObj(const std::string&& key) = 0;
        virtual bool contain(const std::string&& key) = 0;
        virtual std::string toString() = 0;
        virtual std::vector<std::unique_ptr<Parser>> getArray(const std::string&& key) = 0;
        virtual ~Parser() = default;
    };
    class JsonBuilder
    {
    public:
        virtual std::string buildUserMsg(MessageType::User::Type type, std::map<std::string, std::string>&& args) = 0;
        virtual std::string buildSyncMsg(MessageType::Sync::Type type, std::vector<std::string>&& args, uint8_t stride) = 0;
        virtual ~JsonBuilder() = default;
    };
    class JsonFactoryInterface
    {
    protected:
        MessageRegistry registry;
    public:
        JsonFactoryInterface() = default;
        virtual ~JsonFactoryInterface() = default;
        JsonFactoryInterface(const JsonFactoryInterface&) = delete;
        virtual std::unique_ptr<Parser> getParser() = 0;
        virtual std::unique_ptr<JsonBuilder> getBuilder(const BuilderType type) = 0;
    };
};

#endif