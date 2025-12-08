#include "model/ICMPScanner.h"
#include <QtNetwork/QNetworkInterface>
#include <QtCore/QProcess>
#include <QtCore/QRegularExpression>
#include <QtCore/QMutexLocker>
#include <QtCore/QElapsedTimer>
#include <QtCore/QRandomGenerator>
#include <QtNetwork/QHostInfo>

#include "model/DeviceListModel.h"

ICMPScanner::ICMPScanner(QObject *parent)
    : QThread(parent), timeout(200), thread_count(10), stop_scan(false)
{
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

ICMPScanner::~ICMPScanner()
{
    stopScan();
    wait();

#ifdef _WIN32
    WSACleanup();
#endif
}

void ICMPScanner::setNetworkRange(const QString &networkRange)
{
    network_range = networkRange;
}

void ICMPScanner::setTimeout(int timeoutMs)
{
    timeout = timeoutMs;
}

void ICMPScanner::setThreadCount(int count)
{
    thread_count = qMax(1, count);
}

void ICMPScanner::startScan()
{
    stop_scan = false;
    start();
}

void ICMPScanner::stopScan()
{
    stop_scan = true;
}

QVector<DeviceInfo> ICMPScanner::getScanResults() const
{
    QMutexLocker locker(&mutex);
    return scan_results;
}

QVector<QString> ICMPScanner::getLocalNetworks()
{
    if (!networks.empty())
    {
        return networks;
    }

    foreach (const QNetworkInterface &netinterface, QNetworkInterface::allInterfaces())
    {
        // 只启用且非回环的接口
        if (netinterface.flags().testFlag(QNetworkInterface::IsUp) &&
            !netinterface.flags().testFlag(QNetworkInterface::IsLoopBack))
        {

            foreach (const QNetworkAddressEntry &entry, netinterface.addressEntries())
            {
                QHostAddress ip = entry.ip();
                QHostAddress netmask = entry.netmask();

                if (ip.protocol() == QAbstractSocket::IPv4Protocol)
                {
                    // 计算网络地址
                    quint32 network = ip.toIPv4Address() & netmask.toIPv4Address();
                    QHostAddress networkAddr(network);

                    // 计算CIDR
                    int cidr = 0;
                    quint32 mask = netmask.toIPv4Address();
                    while (mask)
                    {
                        cidr += mask & 1;
                        mask >>= 1;
                    }

                    QString networkStr = QString("%1/%2").arg(networkAddr.toString()).arg(cidr);
                    if (!networks.contains(networkStr))
                    {
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
    target_ips.clear();
    scan_results.clear();

    // 解析网络范围
    parseNetworkRange();

    if (target_ips.isEmpty())
    {
        emit scanError("没有找到有效的IP地址范围");
        return;
    }

    emit scanProgress(0);

    QVector<QThread *> workerThreads;
    for (int i = 0; i < thread_count; ++i)
    {
        QThread *thread = QThread::create([this]()
                                          { this->scanWorker(); });
        workerThreads.append(thread);
        thread->start();
    }

    // 等待所有工作线程完成
    for (QThread *thread : workerThreads)
    {
        thread->wait();
        delete thread;
    }

    if (!stop_scan)
    {
        emit scanFinished();
    }
}

void ICMPScanner::parseNetworkRange()
{
    if (network_range.isEmpty())
    {
        // 如果没有设置网络范围，使用所有本地网络
        auto networks = getLocalNetworks();
        for (const QString &network : networks)
        {
            QString baseIP = network.split('/').first();
            QStringList parts = baseIP.split('.');
            if (parts.size() == 4)
            {
                for (int i = 1; i <= 254; ++i)
                {
                    QString target_ip = QString("%1.%2.%3.%4")
                                            .arg(parts[0])
                                            .arg(parts[1])
                                            .arg(parts[2])
                                            .arg(i);
                    if (!local_ip.contains(target_ip))
                        target_ips.append(target_ip);
                }
            }
        }
    }
    else if (network_range.contains('/'))
    {
        // CIDR格式: 192.168.1.0/24
        QStringList parts = network_range.split('/');
        if (parts.size() == 2)
        {
            QString baseIP = parts[0];
            int cidr = parts[1].toInt();

            QStringList ipParts = baseIP.split('.');
            if (ipParts.size() == 4 && cidr >= 24 && cidr <= 32)
            {
                int hostBits = 32 - cidr;
                int hostCount = (1 << hostBits) - 2; // 减去网络地址和广播地址

                for (int i = 1; i <= hostCount && i <= 254; ++i)
                {
                    QString target_ip = QString("%1.%2.%3.%4")
                                            .arg(ipParts[0])
                                            .arg(ipParts[1])
                                            .arg(ipParts[2])
                                            .arg(i);
                    if (!local_ip.contains(target_ip))
                        target_ips.append(target_ip);
                }
            }
        }
    }
    else if (network_range.contains('-'))
    {
        // 范围格式: 192.168.1.1-100
        QStringList parts = network_range.split('-');
        if (parts.size() == 2)
        {
            QString baseIP = parts[0];
            int end = parts[1].toInt();

            QStringList ipParts = baseIP.split('.');
            if (ipParts.size() == 4)
            {
                int start = ipParts[3].toInt();
                for (int i = start; i <= end && i <= 254; ++i)
                {
                    QString target_ip = QString("%1.%2.%3.%4")
                                            .arg(ipParts[0])
                                            .arg(ipParts[1])
                                            .arg(ipParts[2])
                                            .arg(i);
                    if (!local_ip.contains(target_ip))
                        target_ips.append(target_ip);
                }
            }
        }
    }
    else
    {
        // 单个IP
        target_ips.append(network_range);
    }
}

void ICMPScanner::scanWorker()
{
    while (!stop_scan)
    {
        // 随机延迟 [0, 10] 毫秒
        int randomDelay = QRandomGenerator::global()->bounded(0, 11);
        QThread::msleep(randomDelay);

        QString ip;

        {
            QMutexLocker locker(&mutex);
            if (target_ips.isEmpty())
            {
                break;
            }
            ip = target_ips.takeFirst();
        }

        // ICMP探测
        QString hostType;
        bool isOnline = pingHost(ip, hostType);

        if (isOnline)
        {
            DeviceInfo host;
            host.device_ip = ip;
            host.device_type = hostType;
            host.device_name = getComputerName(ip);
            {
                QMutexLocker locker(&mutex);
                scan_results.append(host);
            }
            emit foundOne(host);
        }

        // 计算进度
        int progress = 0;
        {
            QMutexLocker locker(&mutex);
            int total = target_ips.size() + scan_results.size();
            if (total > 0)
            {
                progress = (scan_results.size() * 100) / total;
            }
        }

        emit scanProgress(progress);
    }
}

QString ICMPScanner::getComputerName(const QString &ipAddress)
{
    QString computerName = "Unknown";

    // 首先尝试使用 QHostInfo（跨平台）
    QHostInfo info = QHostInfo::fromName(ipAddress);
    if (info.error() == QHostInfo::NoError && !info.hostName().isEmpty())
    {
        computerName = info.hostName();
        if (computerName != ipAddress)
        {
            return computerName;
        }
    }

#ifdef _WIN32
    sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = inet_addr(ipAddress.toLocal8Bit().constData());

    wchar_t host[NI_MAXHOST];
    if (GetNameInfoW(reinterpret_cast<sockaddr *>(&sa), sizeof(sa),
                     host, NI_MAXHOST, nullptr, 0, NI_NAMEREQD) == 0)
    {
        computerName = QString::fromWCharArray(host);
    }
#else
    sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    if (inet_pton(AF_INET, ipAddress.toLocal8Bit().constData(), &(sa.sin_addr)) <= 0)
    {
        return computerName;
    }

    char host[NI_MAXHOST];
    if (getnameinfo((struct sockaddr *)&sa, sizeof(sa), host, NI_MAXHOST, nullptr, 0, 0) == 0)
    {
        computerName = QString(host);
    }
#endif

    if (computerName == ipAddress)
    {
        computerName = "Unknown";
    }
    return computerName;
}

// 计算ICMP校验和
static uint16_t calculateChecksum(const void *data, int length)
{
    const uint16_t *ptr = (const uint16_t *)data;
    uint32_t sum = 0;

    while (length > 1)
    {
        sum += *ptr++;
        length -= 2;
    }

    if (length == 1)
    {
        sum += *(const uint8_t *)ptr;
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);

    return (uint16_t)~sum;
}

bool ICMPScanner::pingHost(const QString &ipAddress, QString &hostType)
{
#ifdef _WIN32
    HANDLE hIcmp = IcmpCreateFile();
    if (hIcmp == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    const int packetSize = 32;
    char sendData[packetSize];
    memset(sendData, 'A', packetSize);

    const int replySize = sizeof(ICMP_ECHO_REPLY) + packetSize + 8;
    char replyBuffer[replySize];

    ULONG ip = inet_addr(ipAddress.toUtf8().constData());
    if (ip == INADDR_NONE)
    {
        IcmpCloseHandle(hIcmp);
        return false;
    }

    DWORD replies = IcmpSendEcho(hIcmp, ip, sendData, packetSize,
                                 NULL, replyBuffer, replySize, timeout);

    bool success = false;
    if (replies > 0)
    {
        PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)replyBuffer;
        if (pEchoReply->Status == IP_SUCCESS)
        {
            success = true;
            int ttl = pEchoReply->Options.Ttl;
            if (ttl <= 64)
            {
                hostType = "Linux/Unix";
            }
            else if (ttl <= 128)
            {
                hostType = "Windows";
            }
            else
            {
                hostType = "Other";
            }
        }
    }

    IcmpCloseHandle(hIcmp);
    return success;

#else
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0)
    {
        return false;
    }

    // 设置超时
    struct timeval tv;
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // 构造 ICMP 包
    struct icmphdr icmp_hdr;
    memset(&icmp_hdr, 0, sizeof(icmp_hdr));
    icmp_hdr.type = ICMP_ECHO;
    icmp_hdr.code = 0;
    icmp_hdr.un.echo.id = getpid() & 0xFFFF;
    icmp_hdr.un.echo.sequence = 1;

    // 计算校验和
    icmp_hdr.checksum = 0;
    uint16_t checksum = calculateChecksum(&icmp_hdr, sizeof(icmp_hdr));
    icmp_hdr.checksum = checksum;

    // 目标地址
    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, ipAddress.toUtf8().constData(), &dest_addr.sin_addr) <= 0)
    {
        close(sockfd);
        return false;
    }

    // 发送 ICMP 包
    QElapsedTimer timer;
    timer.start();

    ssize_t sent = sendto(sockfd, &icmp_hdr, sizeof(icmp_hdr), 0,
                          (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (sent <= 0)
    {
        close(sockfd);
        return false;
    }

    // 接收回复
    char recv_buffer[1024];
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);

    ssize_t received = recvfrom(sockfd, recv_buffer, sizeof(recv_buffer), 0,
                                (struct sockaddr *)&from_addr, &from_len);

    close(sockfd);

    if (received > 0)
    {
        // 解析 IP 头部
        struct iphdr *ip_hdr = (struct iphdr *)recv_buffer;

        // 计算 ICMP 头部位置（IP头部长度单位是32位字）
        int ip_header_len = ip_hdr->ihl * 4;
        if (received < ip_header_len + (int)sizeof(struct icmphdr))
        {
            return false;
        }

        struct icmphdr *recv_icmp = (struct icmphdr *)(recv_buffer + ip_header_len);

        if (recv_icmp->type == ICMP_ECHOREPLY)
        {
            // 根据 TTL 判断主机类型
            int ttl = ip_hdr->ttl;
            if (ttl <= 64)
            {
                hostType = "Linux/Unix";
            }
            else if (ttl <= 128)
            {
                hostType = "Windows";
            }
            else
            {
                hostType = "Other";
            }
            return true;
        }
    }

    return false;
#endif
}

QString ICMPScanner::getHostTypeFromResponse(const QString &responseData)
{
    // 这里根据TTL、数据包特征等判断操作系统类型
    if (responseData.contains("Windows"))
    {
        return "Windows";
    }
    else if (responseData.contains("Linux"))
    {
        return "Linux";
    }
    else
    {
        return "Unknown";
    }
}

uint32_t ipToUint32(const QString &ip)
{
    struct in_addr addr;
    QByteArray ipBytes = ip.toLocal8Bit();
    if (inet_pton(AF_INET, ipBytes.constData(), &addr) != 1)
    {
        throw std::invalid_argument("Invalid IP format: " + ip.toStdString());
    }
    return ntohl(addr.s_addr);
}

// 根据 CIDR 位数生成掩码
uint32_t cidrToMask(int cidr)
{
    if (cidr < 0 || cidr > 32)
    {
        throw std::invalid_argument("Invalid CIDR: " + std::to_string(cidr));
    }
    return cidr == 0 ? 0 : (0xFFFFFFFFu << (32 - cidr));
}

// 判断 ip 是否在网段 cidr 中
bool ICMPScanner::isIpInCidr(const QString &ip, const QString cidr)
{
    int slashPos = cidr.lastIndexOf('/');
    if (slashPos == -1)
    {
        return false;
    }

    QString networkStr = cidr.left(slashPos);
    bool ok;
    int cidrBits = cidr.mid(slashPos + 1).toInt(&ok);

    if (!ok || cidrBits < 0 || cidrBits > 32)
    {
        return false;
    }

    try
    {
        uint32_t ipInt = ipToUint32(ip);
        uint32_t networkInt = ipToUint32(networkStr);
        uint32_t mask = cidrToMask(cidrBits);

        return (ipInt & mask) == (networkInt & mask);
    }
    catch (const std::exception &)
    {
        return false;
    }
}

QString ICMPScanner::findMatchingLocalIp(const QString &remote_ip)
{
    for (auto it = cidr_ip.constBegin(); it != cidr_ip.constEnd(); ++it)
    {
        if (isIpInCidr(remote_ip, it.key()))
        {
            return it.value();
        }
    }
    return QString();
}

QString ICMPScanner::getLocalComputerName()
{
    QString computerName = QHostInfo::localHostName();

    if (computerName.isEmpty())
    {
        return "Unknown";
    }

    return computerName;
}

void ICMPScanner::refreshLocalNetwork()
{
    networks.clear();
    getLocalNetworks();
}