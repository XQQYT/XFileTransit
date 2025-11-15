#ifndef _CONNECTIONMANAGER_H
#define _CONNECTIONMANAGER_H

#include <QtCore/QObject>

class ConnectionManager : public QObject
{
    Q_OBJECT
public:
    ConnectionManager();
    void onHaveConnectRequest(std::string device_ip, std::string device_name);
    Q_INVOKABLE void accepted(const QString device_ip, const QString device_name);
    Q_INVOKABLE void rejected(const QString device_ip, const QString device_name);
signals:
    void haveConRequest(const QString device_ip, const QString device_name);
private:
    QString current_target_ip;
    QString current_target_name;
    bool current_conn_status;
};


#endif