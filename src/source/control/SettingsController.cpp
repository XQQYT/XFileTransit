#include "control/SettingsController.h"
#include "driver/impl/SettingsFile.h"
#include "control/GlobalStatusManager.h"
#include "control/EventBusManager.h"

SettingsController::SettingsController() : settings_driver(std::make_unique<SettingsFile>())
{
}

void SettingsController::loadSettingsFromFile()
{
    settings_driver->load(Settings::config_file);
    general_config = std::make_shared<std::unordered_map<std::string, std::string>>(
        settings_driver->getConfig(Settings::SettingsGroup::General));

    file_config = std::make_shared<std::unordered_map<std::string, std::string>>(
        settings_driver->getConfig(Settings::SettingsGroup::File));

    transfer_config = std::make_shared<std::unordered_map<std::string, std::string>>(
        settings_driver->getConfig(Settings::SettingsGroup::Transfer));

    notification_config = std::make_shared<std::unordered_map<std::string, std::string>>(
        settings_driver->getConfig(Settings::SettingsGroup::Notification));

    about_config = std::make_shared<std::unordered_map<std::string, std::string>>(
        settings_driver->getConfig(Settings::SettingsGroup::About));

    EventBusManager::instance().subscribe("/settings/get_item_config", std::bind(
                                                                           &SettingsController::onGetConfig,
                                                                           this,
                                                                           std::placeholders::_1));
    EventBusManager::instance().subscribe("/settings/update_settings_value", std::bind(
                                                                                 &SettingsController::onUpdateValue,
                                                                                 this,
                                                                                 std::placeholders::_1,
                                                                                 std::placeholders::_2,
                                                                                 std::placeholders::_3));
    EventBusManager::instance().subscribe("/settings/write_into_file", std::bind(
                                                                           &SettingsController::onFlushConfig,
                                                                           this));
}

void SettingsController::onGetConfig(std::vector<uint8_t> groups)
{
    for (auto i : groups)
    {
        std::shared_ptr<std::unordered_map<std::string, std::string>> result;
        auto group = static_cast<Settings::SettingsGroup>(i);
        switch (group)
        {
        case Settings::SettingsGroup::General:
            result = general_config;
            break;
        case Settings::SettingsGroup::File:
            result = file_config;
            break;
        case Settings::SettingsGroup::Transfer:
            result = transfer_config;
            break;
        case Settings::SettingsGroup::Notification:
            result = notification_config;
            break;
        case Settings::SettingsGroup::About:
            result = about_config;
            break;
        default:
            LOG_ERROR("Invalid group" << i);
            continue;
        }
        EventBusManager::instance().publish("/settings/item_config_reslut", i, result);
    }
}

void SettingsController::onUpdateValue(uint8_t group, std::string key, std::string value)
{
    auto g = static_cast<Settings::SettingsGroup>(group);
    settings_driver->setValue(g, key, value);
    switch (g)
    {
    case Settings::SettingsGroup::General:
        general_config = std::make_shared<std::unordered_map<std::string, std::string>>(
            settings_driver->getConfig(Settings::SettingsGroup::General));
        break;
    case Settings::SettingsGroup::File:
        file_config = std::make_shared<std::unordered_map<std::string, std::string>>(
            settings_driver->getConfig(Settings::SettingsGroup::File));
        break;
    case Settings::SettingsGroup::Transfer:
        transfer_config = std::make_shared<std::unordered_map<std::string, std::string>>(
            settings_driver->getConfig(Settings::SettingsGroup::Transfer));
        break;
    case Settings::SettingsGroup::Notification:
        notification_config = std::make_shared<std::unordered_map<std::string, std::string>>(
            settings_driver->getConfig(Settings::SettingsGroup::Notification));
        break;
    case Settings::SettingsGroup::About:
        about_config = std::make_shared<std::unordered_map<std::string, std::string>>(
            settings_driver->getConfig(Settings::SettingsGroup::About));
        break;
    default:
        LOG_ERROR("Invalid group" << static_cast<int>(g));
        break;
    }
}

void SettingsController::onFlushConfig()
{
    settings_driver->flush();
}