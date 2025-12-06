#include "model/NetworkInfoModel.h"
#include "model/ICMPScanner.h"

NetworkInfoListModel::NetworkInfoListModel(QObject* parent) :
    QAbstractListModel(parent)
{
    syncNetInfoToUI();
}
NetworkInfoListModel::~NetworkInfoListModel()
{

}
int NetworkInfoListModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : net_info_list.size();
}
QVariant NetworkInfoListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= net_info_list.size())
        return QVariant();
    const NetworkInfo& info = net_info_list.at(index.row());
    switch (role)
    {
    case Qt::DisplayRole:
        return "test";
    case Roles::IP:
        return info.ip;
    case Roles::CIDR:
        return info.cidr;
    default:
        return QVariant();
    }
}
QHash<int, QByteArray> NetworkInfoListModel::roleNames() const
{
    static QHash<int, QByteArray> roles = {
        {Roles::IP, "ip"},
        { Roles::CIDR,"cidr" }
    };
    return roles;
}

void NetworkInfoListModel::syncNetInfoToUI()
{
    net_info_list.clear();
    beginResetModel();
    for (auto& i : ICMPScanner::getInstance().getLocalNetworks())
    {
        net_info_list.emplace_back(ICMPScanner::getInstance().getIpByCidr(i), i);
    }
    endResetModel();
}

void NetworkInfoListModel::refreshNetInfo()
{
    ICMPScanner::getInstance().refreshLocalNetwork();
    syncNetInfoToUI();
}