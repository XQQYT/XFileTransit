#ifndef _MODELMANAGER_H
#define _MODELMANAGER_H

#include "model/DeviceListModel.h"
#include "model/FileListModel.h"
#include "model/ConnectionManager.h"
#include <memory>

class ModelManager
{
public:
    static ModelManager& getInstance() {
        static ModelManager  instance;
        return instance;
    }

    std::shared_ptr<DeviceListModel> getDeviceModel() {
        return device_model;
    }

    std::shared_ptr<FileListModel> getFileListModel() {
        return file_list_model;
    }

    std::shared_ptr<ConnectionManager> getConnectionManager() {
        return connection_manager;
    }
private:
    ModelManager() :
        device_model(std::make_shared<DeviceListModel>()),
        file_list_model(std::make_shared<FileListModel>()),
        connection_manager(std::make_shared<ConnectionManager>())
    {
    }
    ModelManager(const ModelManager&) = delete;
    ModelManager& operator=(const ModelManager&) = delete;

private:
    std::shared_ptr<DeviceListModel> device_model;
    std::shared_ptr<FileListModel> file_list_model;
    std::shared_ptr<ConnectionManager> connection_manager;
};

#endif