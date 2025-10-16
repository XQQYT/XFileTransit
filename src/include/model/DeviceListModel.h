#ifndef _DEVICELISTMODEL_H
#define _DEVICELISTMODEL_H

#include <QtCore/QVector>
#include <QtCore/QString>
#include <QtCore/QAbstractListModel>
#include "model/ICMPScanner.h"

struct DeviceInfo
{
    QString device_name;
    QString device_ip;
    QString device_type;
};

class DeviceListModel : public QAbstractListModel
{
    Q_OBJECT
        Q_PROPERTY(bool scanning READ getIsScanning NOTIFY scanningChanged)
public:
    enum Roles
    {
        DeviceName = Qt::UserRole + 1,
        DeviceIP,
        DeviceType
    };

    explicit DeviceListModel(QObject* parent = nullptr);
    ~DeviceListModel();
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void startScan();
    Q_INVOKABLE void stopScan();
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void connectToTarget(const int index);
    bool getIsScanning() { return scanning; }
    quint64 getResultCount() { return device_list.size(); }
signals:
    void scanFinished();
    void scanningChanged();
private slots:
    void onFoundOne(DeviceInfo info);
private:
    void clearAll();
    bool scanning;
    QVector<DeviceInfo> device_list;
    ICMPScanner icmp_scanner;
};

#endif