#ifndef _SETTINGSCONTROLLER_H
#define _SETTINGSCONTROLLER_H

#include "driver/interface/SettingsFileInterface.h"
#include <memory>
#include <stdint.h>

class SettingsController
{
public:
    SettingsController();
    void onGetConfig(std::vector<uint8_t> groups);
    void onUpdateValue(uint8_t group, std::string key, std::string value);

private:
    std::unique_ptr<SettingsFileInterface> settings_driver;
    // 内存中的配置，当调用updateConfig和setValue时才写入文件
    std::shared_ptr<std::unordered_map<std::string, std::string>> general_config;
    std::shared_ptr<std::unordered_map<std::string, std::string>> file_config;
    std::shared_ptr<std::unordered_map<std::string, std::string>> transfer_config;
    std::shared_ptr<std::unordered_map<std::string, std::string>> notification_config;
    std::shared_ptr<std::unordered_map<std::string, std::string>> about_config;
};

#endif