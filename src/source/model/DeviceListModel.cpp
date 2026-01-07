#include "model/DeviceListModel.h"
#include "control/EventBusManager.h"
#include "model/ICMPScanner.h"
#include "control/GlobalStatusManager.h"

DeviceListModel::DeviceListModel(QObject *parent) : QAbstractListModel(parent)
{
    QObject::connect(&ICMPScanner::getInstance(), &ICMPScanner::scanFinished, [this]()
                     {
        emit DeviceListModel::scanFinished();
        scanning = false;
        emit scanningChanged(); });
    EventBusManager::instance().subscribe("/network/have_connect_request_result", [this](bool ret, std::string di)
                                          { QMetaObject::invokeMethod(this, [this, ret, di]()
                                                                      { emit connectResult(ret, QString::fromStdString(di)); }, Qt::QueuedConnection); });
    QObject::connect(&ICMPScanner::getInstance(), &ICMPScanner::foundOne, this, &DeviceListModel::onFoundOne);
    QObject::connect(&ICMPScanner::getInstance(), &ICMPScanner::scanProgress, this, [=](int progress)
                     { emit scanProgress(progress); });
}
DeviceListModel::~DeviceListModel()
{
}
int DeviceListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : device_list.size();
}
QVariant DeviceListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= device_list.size())
        return QVariant();
    const DeviceInfo &info = device_list.at(index.row());
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
        {DeviceIP, "deviceIP"},
        {DeviceType, "deviceType"}};
    return roles;
}
void DeviceListModel::startScan()
{
    clearAll();
    ICMPScanner::getInstance().startScan();
    scanning = true;
    emit scanningChanged();
}
void DeviceListModel::stopScan()
{
    ICMPScanner::getInstance().stopScan();
    scanning = false;
    emit scanningChanged();
}

void DeviceListModel::onFoundOne(DeviceInfo info)
{
    // 检查是否已存在该设备
    for (const auto &existingDevice : device_list)
    {
        if (existingDevice.device_ip == info.device_ip)
        {
            return; // 已存在，不重复添加
        }
    }

    // 在列表末尾插入新设备
    beginInsertRows(QModelIndex(), device_list.count(), device_list.count());
    device_list.push_back(info);
    endInsertRows();

    // qDebug() << "发现新设备:" << info.device_name << "IP:" << info.device_ip;
}
void DeviceListModel::refresh()
{
    clearAll();
    startScan();
}
void DeviceListModel::clearAll()
{
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
                                        ICMPScanner::getInstance().getLocalComputerName().toStdString(),
                                        ICMPScanner::getInstance().findMatchingLocalIp(device.device_ip).toStdString(),
                                        device.device_ip.toStdString());
}

void DeviceListModel::connectToTarget(const QString ip)
{
    ConnectionInfo::connection_type = ConnectionInfo::Type::Tcp;
    std::unordered_map<std::string, std::string> args;
    args["sender_name"] = ICMPScanner::getInstance().getLocalComputerName().toStdString();
    args["sender_ip"] = ICMPScanner::getInstance().findMatchingLocalIp(ip).toStdString();
    args["target_ip"] = ip.toStdString();
    EventBusManager::instance().publish("/network/send_connect_request", args);
}
/*
cmake .. -G "MinGW Makefiles" -DCMAKE_C_COMPILER=D:/Qt6.8/Tools/mingw1310_64/bin/gcc.exe -DCMAKE_CXX_COMPILER=D:/Qt6.8/Tools/mingw1310_64/bin/g++.exe -DCMAKE_MAKE_PROGRAM=D:/Qt6.8/Tools/mingw1310_64/bin/mingw32-make.exe -DUSE_GNUTLS=0 -DUSE_SYSTEM_SRTP=ON -DUSE_SYSTEM_USRSCTP=ON -DBUILD_SHARED_LIBS=ON
*/
void DeviceListModel::resetConnection()
{
    EventBusManager::instance().publish("/network/reset_connection");
}

bool DeviceListModel::isLocalIp(const QString ip)
{
    return ICMPScanner::getInstance().isLocalAddress(ip);
}

void DeviceListModel::connectViaP2P(const QString code, const QString password)
{
    ConnectionInfo::connection_type = ConnectionInfo::Type::P2P;
    std::unordered_map<std::string, std::string>
        args;
    args["code"] = code.toStdString();
    args["password"] = password.toStdString();
    EventBusManager::instance().publish("/network/send_connect_request", args);
}