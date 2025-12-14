#include "model/ModelManager.h"

ModelManager::ModelManager()
    : net_info_model(std::make_shared<NetworkInfoListModel>()),
      device_model(std::make_shared<DeviceListModel>()),
      file_list_model(std::make_shared<FileListModel>()),
      connection_manager(std::make_shared<ConnectionManager>()),
      settings_model(std::make_shared<SettingsModel>())
{
    connect(connection_manager.get(), &ConnectionManager::connectionClosed,
            file_list_model.get(), &FileListModel::onConnectionClosed);
    connect(settings_model.get(), &SettingsModel::settingsChanged,
            file_list_model.get(), &FileListModel::onSettingsChanged);
}

ModelManager &ModelManager::getInstance()
{
    static ModelManager instance;
    return instance;
}

std::shared_ptr<NetworkInfoListModel> ModelManager::getNetworkInfoModel() { return net_info_model; }
std::shared_ptr<DeviceListModel> ModelManager::getDeviceModel() { return device_model; }
std::shared_ptr<FileListModel> ModelManager::getFileListModel() { return file_list_model; }
std::shared_ptr<ConnectionManager> ModelManager::getConnectionManager() { return connection_manager; }
std::shared_ptr<SettingsModel> ModelManager::getSettingsModel() { return settings_model; }
