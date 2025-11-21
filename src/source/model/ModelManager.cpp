#include "model/ModelManager.h"

ModelManager::ModelManager()
    : device_model(std::make_shared<DeviceListModel>()),
      file_list_model(std::make_shared<FileListModel>()),
      connection_manager(std::make_shared<ConnectionManager>())
{
    connect(connection_manager.get(), &ConnectionManager::connectionClosed,
            file_list_model.get(), &FileListModel::onConnectionClosed);
}

ModelManager& ModelManager::getInstance() {
    static ModelManager instance;
    return instance;
}

std::shared_ptr<DeviceListModel> ModelManager::getDeviceModel() { return device_model; }
std::shared_ptr<FileListModel> ModelManager::getFileListModel() { return file_list_model; }
std::shared_ptr<ConnectionManager> ModelManager::getConnectionManager() { return connection_manager; }
