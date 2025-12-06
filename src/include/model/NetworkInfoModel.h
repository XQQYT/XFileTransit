#ifndef _NETWORKINFOMODEL_H
#define _NETWORKINFOMODEL_H

#include <QtCore/QVector>
#include <QtCore/QString>
#include <QtCore/QAbstractListModel>

struct NetworkInfo
{
    QString ip;
    QString cidr;
    NetworkInfo(const QString a_ip, const QString a_cidr) :
        ip(a_ip), cidr(a_cidr) {
    }
};

class NetworkInfoListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles
    {
        IP = Qt::UserRole + 1,
        CIDR
    };

    explicit NetworkInfoListModel(QObject* parent = nullptr);
    ~NetworkInfoListModel();
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void refreshNetInfo();
    Q_INVOKABLE void copyNetInfoText();
private:
    void syncNetInfoToUI();
private:
    QList<NetworkInfo> net_info_list;
};

#endif