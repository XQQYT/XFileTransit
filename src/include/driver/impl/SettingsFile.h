#ifndef _SETTINGSFILE_H
#define _SETTINGSFILE_H

#include "driver/interface/SettingsFileInterface.h"
#include "nlohmann/json.hpp"
#include <fstream>

using json = nlohmann::json;

class SettingsFile : public SettingsFileInterface
{
public:
    void load(const std::string &path) override;
    std::unordered_map<std::string, std::string> getConfig(const Settings::SettingsGroup config_name) override;
    void updateConfig(const Settings::SettingsGroup config_name, const std::unordered_map<std::string, std::string> config) override;
    void setValue(const Settings::SettingsGroup config_name, const std::string key, const std::string value) override;
    void flush() override;

private:
    json full_config;
    std::unique_ptr<std::ofstream> out;
    std::string config_path;
};

#endif