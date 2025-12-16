#ifndef _SETTINGFILEINTERFACE_H
#define _SETTINGFILEINTERFACE_H

#include <string>
#include <unordered_map>

#include "control/GlobalStatusManager.h"

class SettingsFileInterface
{
public:
    virtual void load(const std::string &path) = 0;
    virtual std::unordered_map<std::string, std::string> getConfig(const Settings::SettingsGroup config_name) = 0;
    virtual void updateConfig(const Settings::SettingsGroup config_name, const std::unordered_map<std::string, std::string> config) = 0;
    virtual void setValue(const Settings::SettingsGroup config_name, const std::string key, const std::string value) = 0;
    virtual ~SettingsFileInterface() = default;
};

#endif