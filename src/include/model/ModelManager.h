#ifndef _MODELMANAGER_H
#define _MODELMANAGER_H

#include <QObject>
#include <memory>

#include "model/DeviceListModel.h"
#include "model/FileListModel.h"
#include "model/ConnectionManager.h"

class ModelManager : public QObject
{
    Q_OBJECT
public:
    static ModelManager& getInstance();

    std::shared_ptr<DeviceListModel> getDeviceModel();
    std::shared_ptr<FileListModel> getFileListModel();
    std::shared_ptr<ConnectionManager> getConnectionManager();

private:
    ModelManager();
    ModelManager(const ModelManager&) = delete;
    ModelManager& operator=(const ModelManager&) = delete;
    ~ModelManager() = default;

private:
    std::shared_ptr<DeviceListModel> device_model;
    std::shared_ptr<FileListModel> file_list_model;
    std::shared_ptr<ConnectionManager> connection_manager;
};

#endif
