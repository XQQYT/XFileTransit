#include "driver/impl/SettingsFile.h"
#include "common/DebugOutputer.h"

void SettingsFile::load(const std::string &path)
{
    config_path = path;
    std::ifstream in(path);
    if (!in.is_open())
    {
        LOG_ERROR("Cann't open config file" << path);
        in.close();
    }
    full_config = json::parse(in);
    out = std::make_unique<std::ofstream>(path);
    writeFullJson();
}

std::unordered_map<std::string, std::string> SettingsFile::getConfig(const Settings::SettingsGroup config_name)
{
    std::string config_name_str = Settings::toString(config_name);

    if (!full_config.contains(config_name_str))
    {
        LOG_ERROR("Cann't find config: " << config_name_str);
        return {};
    }
    return full_config[config_name_str].get<std::unordered_map<std::string, std::string>>();
}

void SettingsFile::writeFullJson()
{
    out->close();
    out = std::make_unique<std::ofstream>(config_path);
    std::string full_json_str = full_config.dump();
    out->write(full_json_str.data(), full_json_str.size());
    out->flush();
}
void SettingsFile::updateConfig(const Settings::SettingsGroup config_name, const std::unordered_map<std::string, std::string> config)
{
    std::string config_name_str = Settings::toString(config_name);
    if (!full_config.contains(config_name_str))
    {
        LOG_ERROR("Cann't find config: " << config_name_str);
        return;
    }
    full_config[config_name_str] = config;
    writeFullJson();
}

void SettingsFile::setValue(const Settings::SettingsGroup config_name, const std::string key, const std::string value)
{
    std::string config_name_str = Settings::toString(config_name);
    if (!full_config.contains(config_name_str))
    {
        LOG_ERROR("Cann't find config: " << config_name_str);
        return;
    }
    json config_json = full_config[config_name_str];
    if (!config_json.contains(key))
    {
        LOG_ERROR("Cann't find key: " << key);
        return;
    }
    config_json[key] = value;
    full_config[config_name_str] = config_json;
    writeFullJson();
}