#ifndef ICMPSCANNER_H
#define ICMPSCANNER_H

#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtNetwork/QHostInfo>
#include <QtCore/QVector>
#include <QtCore/QSet>
#include <QtCore/QMutex>
#include "model/DeviceListModel.h"
#include "driver/interface/PlatformSocket.h"

class ICMPScanner : public QThread
{
    Q_OBJECT

public:
    ~ICMPScanner();
    static ICMPScanner &getInstance()
    {
        static ICMPScanner instance;
        return instance;
    }
    void setNetworkRange(const QString &networkRange);
    void setTimeout(int timeoutMs);
    void setThreadCount(int count);

    void startScan();
    void stopScan();

    QVector<DeviceInfo> getScanResults() const;
    QVector<QString> getLocalNetworks();
    QString findMatchingLocalIp(const QString &remote_ip);
    QString getLocalComputerName();
    void refreshLocalNetwork();
    QString getIpByCidr(const QString &cidr)
    {
        auto i = cidr_ip.find(cidr);
        if (i != cidr_ip.end())
        {
            return *i;
        }
        return "unknow";
    }
    // 判断是否为本机地址
    inline bool isLocalAddress(const QString ip)
    {
        return local_ip.contains(ip);
    }
signals:
    void foundOne(const DeviceInfo &host);
    void scanProgress(int progress);
    void scanFinished();
    void scanError(const QString &error);

protected:
    void run() override;

private:
    void parseNetworkRange();
    void scanWorker();
    bool pingHost(const QString &ipAddress, QString &hostType);
    QString getComputerName(const QString &ipAddress);
    QString getHostTypeFromResponse(const QString &responseData);
    bool isIpInCidr(const QString &ip, const QString cidr);

private:
    ICMPScanner(QObject *parent = nullptr);
    QString network_range;
    int timeout;
    int thread_count;
    bool stop_scan;

    mutable QMutex mutex;
    QVector<QString> target_ips;
    QVector<DeviceInfo> scan_results;
    QSet<QString> local_ip;
    QVector<QString> networks;
    QMap<QString, QString> cidr_ip;
};

uint32_t ipToUint32(const QString &ip);
uint32_t cidrToMask(int cidr);

#endif // ICMPSCANNER_H