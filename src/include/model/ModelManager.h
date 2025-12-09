#ifndef _MODELMANAGER_H
#define _MODELMANAGER_H

#include <QtCore/QObject>
#include <memory>

#include "model/DeviceListModel.h"
#include "model/FileListModel.h"
#include "model/ConnectionManager.h"
#include "model/NetworkInfoModel.h"
#include "model/SettingsModel.h"

class ModelManager : public QObject
{
    Q_OBJECT
public:
    static ModelManager &getInstance();

    std::shared_ptr<NetworkInfoListModel> getNetworkInfoModel();
    std::shared_ptr<DeviceListModel> getDeviceModel();
    std::shared_ptr<FileListModel> getFileListModel();
    std::shared_ptr<ConnectionManager> getConnectionManager();
    std::shared_ptr<SettingsModel> getSettingsModel();

private:
    ModelManager();
    ModelManager(const ModelManager &) = delete;
    ModelManager &operator=(const ModelManager &) = delete;
    ~ModelManager() = default;

private:
    std::shared_ptr<NetworkInfoListModel> net_info_model;
    std::shared_ptr<DeviceListModel> device_model;
    std::shared_ptr<FileListModel> file_list_model;
    std::shared_ptr<ConnectionManager> connection_manager;
    std::shared_ptr<SettingsModel> settings_model;
};

#endif
