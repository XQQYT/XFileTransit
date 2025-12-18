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
    std::ofstream out(config_path, std::ios::trunc);
    if (!out.is_open())
    {
        LOG_ERROR("Cannot open config file for writing: " << config_path);
        return;
    }
    std::string full_json_str = full_config.dump();
    out << full_json_str;
    out.close();
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