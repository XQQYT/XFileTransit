#ifndef _JSONFACTORY_H
#define _JSONFACTORY_H

#include <string>
#include <vector>
#include <memory>
#include <map>

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
    virtual std::string build(std::map<std::string, std::string>& args) = 0;
    virtual ~JsonBuilder() = default;
};
class JsonFactoryInterface
{
public:
    enum class MsgType
    {
        User,
        File,
        Sync
    };
    JsonFactoryInterface() = default;
    virtual ~JsonFactoryInterface() = default;
    JsonFactoryInterface(const JsonFactoryInterface&) = delete;
    virtual std::unique_ptr<Parser> getParser() = 0;
    virtual std::unique_ptr<JsonBuilder> getBuilder(const MsgType type) = 0;
};

#endif