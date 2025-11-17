#ifndef _CONNECTIONMANAGER_H
#define _CONNECTIONMANAGER_H

#include <QtCore/QObject>

class ConnectionManager : public QObject
{
    Q_OBJECT
public:
    ConnectionManager();
    void onHaveConnectRequest(std::string device_ip, std::string device_name);
    void onHaveConnectError(std::string message);
    void onHaveRecvError(std::string message);
    void onConnectionClosed();
    Q_INVOKABLE void accepted(const QString device_ip, const QString device_name);
    Q_INVOKABLE void rejected(const QString device_ip, const QString device_name);
    Q_INVOKABLE void disconnect();
signals:
    void haveConRequest(const QString device_ip, const QString device_name);
    void haveConnectError(QString message);
    void haveRecvError(QString message);
    void connectionClosed();
};


#endif