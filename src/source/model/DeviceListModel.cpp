#include "model/DeviceListModel.h"
#include "control/EventBusManager.h"

DeviceListModel::DeviceListModel(QObject* parent)
{
    QObject::connect(&icmp_scanner, &ICMPScanner::scanFinished, [this]() {
        emit DeviceListModel::scanFinished();
        scanning = false;
        emit scanningChanged();
        });
    QObject::connect(&icmp_scanner, &ICMPScanner::foundOne, this, &DeviceListModel::onFoundOne);
}
DeviceListModel::~DeviceListModel()
{

}
int DeviceListModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : device_list.size();
}
QVariant DeviceListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= device_list.size())
        return QVariant();
    const DeviceInfo& info = device_list.at(index.row());
    switch (role)
    {
    case Qt::DisplayRole:
        return info.device_name.isNull() ? info.device_ip : info.device_name;
    case DeviceName:
        return info.device_name;
    case DeviceIP:
        return info.device_ip;
    case DeviceType:
        return info.device_type;
    default:
        return QVariant();
    }
}
QHash<int, QByteArray> DeviceListModel::roleNames() const
{
    static QHash<int, QByteArray> roles = {
        {DeviceName, "deviceName"},
        { DeviceIP,"deviceIP" },
        { DeviceType,"deviceType" }
    };
    return roles;
}
void DeviceListModel::startScan()
{
    clearAll();
    icmp_scanner.startScan();
    scanning = true;
    emit scanningChanged();
}
void DeviceListModel::stopScan()
{
    icmp_scanner.stopScan();
    scanning = false;
    emit scanningChanged();
}

void DeviceListModel::onFoundOne(DeviceInfo info)
{
    // 检查是否已存在该设备
    for (const auto& existingDevice : device_list)
    {
        if (existingDevice.device_ip == info.device_ip) {
            return; // 已存在，不重复添加
        }
    }

    // 在列表末尾插入新设备
    beginInsertRows(QModelIndex(), device_list.count(), device_list.count());
    device_list.push_back(info);
    endInsertRows();

    qDebug() << "发现新设备:" << info.device_name << "IP:" << info.device_ip;
}
void DeviceListModel::refresh()
{
    clearAll();
    startScan();
}
void DeviceListModel::clearAll() {
    if (device_list.isEmpty())
    {
        return;
    }
    beginResetModel();
    device_list.clear();
    endResetModel();
}
void DeviceListModel::connectToTarget(const int index)
{
    if (index < 0 || index >= device_list.size())
        return;
    auto device = device_list.at(index);
    EventBusManager::instance().publish("/network/send_connect_request",
        device.device_name.toStdString(), device.device_ip.toStdString());
    qDebug() << tr("本地ip为") << icmp_scanner.findMatchingLocalIp(device.device_ip);
}