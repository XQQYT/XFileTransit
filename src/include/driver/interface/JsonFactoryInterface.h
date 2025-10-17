#ifndef _JSONFACTORY_H
#define _JSONFACTORY_H

#include <string>
#include <vector>
#include <memory>
#include <map>

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
            };

            constexpr const char* toString(Type type)
            {
                switch (type)
                {
                case ConnectRequest: return "user_connect_request";
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
            // enum Type
            // {

            // };

            // constexpr const char* toString(Type type)
            // {

            // }
        }
    }

    // 消息架构描述
    struct MessageSchema
    {
        uint64_t type;
        std::string type_name;
        std::vector<std::string> required_fields;
        std::vector<std::string> optional_fields;
    };

    // 消息注册表
    class MessageRegistry
    {
    private:
        std::unordered_map<int, MessageSchema> schemas;

    public:
        MessageRegistry()
        {
            registerSchema(MessageType::User::ConnectRequest, "connect_request",
                { "sender_device_name", "sender_device_ip" }, {});
        }

        void registerSchema(uint64_t type, const std::string& type_name,
            const std::vector<std::string>& required,
            const std::vector<std::string>& optional)
        {
            schemas[type] = { type, type_name, required, optional };
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
        virtual std::string getValue(const std::string& key) = 0;
        virtual std::unique_ptr<Parser> getObj(const std::string& key) = 0;
        virtual bool contain(const std::string& key) = 0;
        virtual std::string toString() = 0;
        virtual std::vector<std::unique_ptr<Parser>> getArray(const std::string& key) = 0;
        virtual ~Parser() = default;
    };
    class JsonBuilder
    {
    public:
        virtual std::string build(uint64_t type, std::map<std::string, std::string>& args) = 0;
        virtual std::string build(uint64_t type, std::map<std::string, std::string>&& args) = 0;
        std::string buildImpl(uint64_t type, std::map<std::string, std::string>&& args) { return "please refence buildImpl template in drived class"; }
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