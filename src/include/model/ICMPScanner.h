#ifndef ICMPSCANNER_H
#define ICMPSCANNER_H

#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtNetwork/QHostInfo>
#include <QtCore/QVector>
#include <QtCore/QSet>
#include <QtCore/QMutex>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <icmpapi.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

struct DeviceInfo;
// ICMP扫描器类
class ICMPScanner : public QThread
{
    Q_OBJECT

public:
    explicit ICMPScanner(QObject* parent = nullptr);
    ~ICMPScanner();

    // 设置扫描的网络段 (如: "192.168.1.0/24" 或 "192.168.1.1-100")
    void setNetworkRange(const QString& networkRange);

    // 设置超时时间(毫秒)
    void setTimeout(int timeoutMs);

    // 设置线程数 (用于并行扫描)
    void setThreadCount(int count);

    // 开始扫描
    void startScan();

    // 停止扫描
    void stopScan();

    // 获取扫描结果
    QVector<DeviceInfo> getScanResults() const;

    //判断ip是否在某一网段cidr
    bool isIpInCidr(const QString& ip, const QString cidr);

    QString findMatchingLocalIp(const QString& remote_ip);

signals:
    // 扫描进度信号 (0-100)
    void scanProgress(int percent);

    // 扫描完成信号
    void scanFinished();

    // 扫描错误信号
    void scanError(const QString& error);

    //发现一个主机
    void foundOne(DeviceInfo info);
protected:
    void run() override;

private:
    // 获取本地网络接口信息
    QVector<QString> getLocalNetworks();

    // ICMP探测接口 - 需要您实现这个函数
    bool pingHost(const QString& ipAddress, QString& hostType);

    // 解析网络范围
    void parseNetworkRange();

    // 工作线程函数
    void scanWorker();

    // 获取主机类型
    QString getHostTypeFromResponse(const QString& responseData);

    QString getComputerName(const QString& ipAddress);


    QString m_networkRange;
    int m_timeout;
    int m_threadCount;
    bool m_stopScan;

    QVector<QString> m_targetIPs;
    QVector<DeviceInfo> m_scanResults;
    QSet<QString> m_pendingLookups;
    mutable QMutex m_mutex;

    QSet<QString> local_ip;
    QMap<QString, QString> cidr_ip;
};

#endif // ICMPSCANNER_H