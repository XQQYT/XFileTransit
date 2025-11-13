#ifndef _CONNECTIONMANAGER_H
#define _CONNECTIONMANAGER_H

#include <QtCore/QObject>

class ConnectionManager : public QObject
{
    Q_OBJECT
public:
    ConnectionManager();
    void onHaveConnectRequest(std::string device_ip, std::string device_name);
signals:
    void haveConRequest(const QString device_ip, const QString device_name);
};


#endif