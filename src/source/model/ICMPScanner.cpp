#include "model/ICMPScanner.h"
#include <QtNetwork/QNetworkInterface>
#include <QtCore/QProcess>
#include <QtCore/QRegularExpression>
#include <QtCore/QMutexLocker>
#include <QtCore/QElapsedTimer>
#include <QtCore/QRandomGenerator>

#include "model/DeviceListModel.h"

ICMPScanner::ICMPScanner(QObject* parent)
    : QThread(parent)
    , m_timeout(500)
    , m_threadCount(10)
    , m_stopScan(false)
{
}

ICMPScanner::~ICMPScanner()
{
    stopScan();
    WSACleanup();
    wait();
}

void ICMPScanner::setNetworkRange(const QString& networkRange)
{
    m_networkRange = networkRange;
}

void ICMPScanner::setTimeout(int timeoutMs)
{
    m_timeout = timeoutMs;
}

void ICMPScanner::setThreadCount(int count)
{
    m_threadCount = qMax(1, count);
}

void ICMPScanner::startScan()
{
    m_stopScan = false;
    start();
}

void ICMPScanner::stopScan()
{
    m_stopScan = true;
}

QVector<DeviceInfo> ICMPScanner::getScanResults() const
{
    QMutexLocker locker(&m_mutex);
    return m_scanResults;
}

QVector<QString> ICMPScanner::getLocalNetworks()
{
    if (!networks.empty())
    {
        return networks;
    }

    foreach(const QNetworkInterface & netinterface, QNetworkInterface::allInterfaces()) {
        // 只启用且非回环的接口
        if (netinterface.flags().testFlag(QNetworkInterface::IsUp) &&
            !netinterface.flags().testFlag(QNetworkInterface::IsLoopBack)) {

            foreach(const QNetworkAddressEntry & entry, netinterface.addressEntries()) {
                QHostAddress ip = entry.ip();
                QHostAddress netmask = entry.netmask();

                if (ip.protocol() == QAbstractSocket::IPv4Protocol) {
                    // 计算网络地址
                    quint32 network = ip.toIPv4Address() & netmask.toIPv4Address();
                    QHostAddress networkAddr(network);

                    // 计算CIDR
                    int cidr = 0;
                    quint32 mask = netmask.toIPv4Address();
                    while (mask) {
                        cidr += mask & 1;
                        mask >>= 1;
                    }

                    QString networkStr = QString("%1/%2").arg(networkAddr.toString()).arg(cidr);
                    if (!networks.contains(networkStr)) {
                        local_ip.insert(ip.toString());
                        cidr_ip[networkStr] = ip.toString();
                        networks.append(networkStr);
                    }
                }
            }
        }
    }
    return networks;
}

void ICMPScanner::run()
{
    m_targetIPs.clear();
    m_scanResults.clear();

    // 解析网络范围
    parseNetworkRange();

    if (m_targetIPs.isEmpty()) {
        emit scanError("没有找到有效的IP地址范围");
        return;
    }

    emit scanProgress(0);

    // 创建多个工作线程
    QVector<QThread*> workerThreads;
    for (int i = 0; i < m_threadCount; ++i) {
        QThread* thread = QThread::create([this]() { this->scanWorker(); });
        workerThreads.append(thread);
        thread->start();
    }

    // 等待所有工作线程完成
    for (QThread* thread : workerThreads) {
        thread->wait();
        delete thread;
    }

    if (!m_stopScan) {
        emit scanFinished();
    }
}

void ICMPScanner::parseNetworkRange()
{
    if (m_networkRange.isEmpty()) {
        // 如果没有设置网络范围，使用所有本地网络
        auto networks = getLocalNetworks();
        for (const QString& network : networks) {
            QString baseIP = network.split('/').first();
            QStringList parts = baseIP.split('.');
            if (parts.size() == 4) {
                for (int i = 1; i <= 254; ++i) {
                    QString target_ip = QString("%1.%2.%3.%4")
                        .arg(parts[0]).arg(parts[1]).arg(parts[2]).arg(i);
                    if (!local_ip.contains(target_ip))
                        m_targetIPs.append(target_ip);
                }
            }
        }
    }
    else if (m_networkRange.contains('/')) {
        // CIDR格式: 192.168.1.0/24
        QStringList parts = m_networkRange.split('/');
        if (parts.size() == 2) {
            QString baseIP = parts[0];
            int cidr = parts[1].toInt();

            QStringList ipParts = baseIP.split('.');
            if (ipParts.size() == 4 && cidr >= 24 && cidr <= 32) {
                int hostBits = 32 - cidr;
                int hostCount = (1 << hostBits) - 2; // 减去网络地址和广播地址

                for (int i = 1; i <= hostCount && i <= 254; ++i) {
                    QString target_ip = QString("%1.%2.%3.%4")
                        .arg(ipParts[0]).arg(ipParts[1]).arg(ipParts[2]).arg(i);
                    if (!local_ip.contains(target_ip))
                        m_targetIPs.append(target_ip);
                }
            }
        }
    }
    else if (m_networkRange.contains('-')) {
        // 范围格式: 192.168.1.1-100
        QStringList parts = m_networkRange.split('-');
        if (parts.size() == 2) {
            QString baseIP = parts[0];
            int end = parts[1].toInt();

            QStringList ipParts = baseIP.split('.');
            if (ipParts.size() == 4) {
                int start = ipParts[3].toInt();
                for (int i = start; i <= end && i <= 254; ++i) {
                    QString target_ip = QString("%1.%2.%3.%4")
                        .arg(ipParts[0]).arg(ipParts[1]).arg(ipParts[2]).arg(i);
                    if (!local_ip.contains(target_ip))
                        m_targetIPs.append(target_ip);
                }
            }
        }
    }
    else {
        // 单个IP
        m_targetIPs.append(m_networkRange);
    }
}

void ICMPScanner::scanWorker()
{
    while (!m_stopScan) {
        // 随机延迟 [0, 10] 毫秒
        int randomDelay = QRandomGenerator::global()->bounded(0, 11);
        QThread::msleep(randomDelay);

        QString ip;

        {
            QMutexLocker locker(&m_mutex);
            if (m_targetIPs.isEmpty()) {
                break;
            }
            ip = m_targetIPs.takeFirst();
        }

        // ICMP探测
        QString hostType;
        bool isOnline = pingHost(ip, hostType);

        if (isOnline) {
            DeviceInfo host;
            host.device_ip = ip;
            host.device_type = hostType;
            host.device_name = getComputerName(ip);
            {
                QMutexLocker locker(&m_mutex);
                m_scanResults.append(host);
            }
            emit foundOne(host);
        }

        // 计算进度
        int progress = 0;
        {
            QMutexLocker locker(&m_mutex);
            int total = m_targetIPs.size() + m_scanResults.size();
            if (total > 0) {
                progress = (m_scanResults.size() * 100) / total;
            }
        }

        emit scanProgress(progress);

    }
}

QString ICMPScanner::getComputerName(const QString& ipAddress)
{
    QString computerName = "Unknown";
    do {
        //尝试使用反向代理
        QHostInfo info = QHostInfo::fromName(ipAddress);
        if (info.error() == QHostInfo::NoError && !info.hostName().isEmpty()) {
            computerName = info.hostName();
            break;
        }

        //尝试使用window api
        sockaddr_in sa;
        memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = inet_addr(ipAddress.toLocal8Bit().constData());
        wchar_t host[NI_MAXHOST];
        if (GetNameInfoW(reinterpret_cast<sockaddr*>(&sa), sizeof(sa),
            host, NI_MAXHOST, nullptr, 0, NI_NAMEREQD) == 0) {
            computerName = QString::fromWCharArray(host);
            break;
        }
    } while (false);

    if (computerName == ipAddress) {
        computerName = "Unknow";
    }
    return computerName;
}

bool ICMPScanner::pingHost(const QString& ipAddress, QString& hostType)
{
    // 初始化Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return false;
    }

    // 创建ICMP句柄
    HANDLE hIcmp = IcmpCreateFile();
    if (hIcmp == INVALID_HANDLE_VALUE) {
        WSACleanup();
        return false;
    }

    // 准备发送数据
    const int packetSize = 32;
    char sendData[packetSize];
    memset(sendData, 'A', packetSize);  // 填充测试数据

    // 准备接收缓冲区
    const int replySize = sizeof(ICMP_ECHO_REPLY) + packetSize;
    char replyBuffer[replySize];

    // 解析目标IP地址
    ULONG ip = inet_addr(ipAddress.toUtf8().constData());
    if (ip == INADDR_NONE) {
        IcmpCloseHandle(hIcmp);
        WSACleanup();
        return false;
    }

    // 发送ICMP Echo请求
    DWORD replies = IcmpSendEcho(hIcmp, ip, sendData, packetSize,
        NULL, replyBuffer, replySize, m_timeout);

    bool success = false;
    if (replies > 0) {
        PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)replyBuffer;

        if (pEchoReply->Status == IP_SUCCESS) {
            success = true;

            // 根据TTL判断主机类型
            int ttl = pEchoReply->Options.Ttl;
            if (ttl <= 64) {
                hostType = "Linux/Unix";
            }
            else if (ttl <= 128) {
                hostType = "Windows";
            }
            else {
                hostType = "Other";
            }
        }
    }

    // 清理资源
    IcmpCloseHandle(hIcmp);

    return success;
}

QString ICMPScanner::getHostTypeFromResponse(const QString& responseData)
{
    // TODO: 根据ICMP响应数据解析主机类型
    // 这里可以根据TTL、数据包特征等判断操作系统类型

    // 临时实现
    if (responseData.contains("Windows")) {
        return "Windows";
    }
    else if (responseData.contains("Linux")) {
        return "Linux";
    }
    else {
        return "Unknown";
    }
}

uint32_t ipToUint32(const QString& ip) {
    struct in_addr addr;
    QByteArray ipBytes = ip.toLocal8Bit();
    if (inet_pton(AF_INET, ipBytes.constData(), &addr) != 1) {
        throw std::invalid_argument("Invalid IP format: " + ip.toStdString());
    }
    return ntohl(addr.s_addr);
}

// 根据 CIDR 位数生成掩码
uint32_t cidrToMask(int cidr) {
    if (cidr < 0 || cidr > 32) {
        throw std::invalid_argument("Invalid CIDR: " + std::to_string(cidr));
    }
    return cidr == 0 ? 0 : (0xFFFFFFFFu << (32 - cidr));
}

// 判断 ip 是否在网段 cidr 中
bool ICMPScanner::isIpInCidr(const QString& ip, const QString cidr) {
    int slashPos = cidr.lastIndexOf('/');
    if (slashPos == -1) {  // 使用 -1 而不是 std::string::npos
        return false;
    }

    QString networkStr = cidr.left(slashPos);  // 使用 left 而不是 mid(0, slashPos)
    bool ok;
    int cidrBits = cidr.mid(slashPos + 1).toInt(&ok);

    if (!ok || cidrBits < 0 || cidrBits > 32) {
        return false;
    }

    try {
        uint32_t ipInt = ipToUint32(ip);
        uint32_t networkInt = ipToUint32(networkStr);
        uint32_t mask = cidrToMask(cidrBits);

        return (ipInt & mask) == (networkInt & mask);
    }
    catch (const std::exception&) {
        return false;
    }
}
QString ICMPScanner::findMatchingLocalIp(const QString& remote_ip)
{
    for (auto it = cidr_ip.constBegin(); it != cidr_ip.constEnd(); ++it)
    {
        if (isIpInCidr(remote_ip, it.key())) {
            return it.value();
        }
    }
    return QString();
}

QString ICMPScanner::getLocalComputerName()
{
    QString computerName = QHostInfo::localHostName();

    if (computerName.isEmpty()) {
        return "Unknown";
    }

    return computerName;
}

void ICMPScanner::refreshLocalNetwork()
{
    networks.clear();
    getLocalNetworks();
}
