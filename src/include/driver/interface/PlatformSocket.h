#ifndef PLATFORM_SOCKET_H
#define PLATFORM_SOCKET_H

#ifdef _WIN32
// Windows头文件
#define WIN32_LEAN_AND_MEAN

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00 // Windows 10
#endif

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#include <iphlpapi.h>
#include <icmpapi.h>

#define SOCKET_TYPE SOCKET
#define SOCKET_ERROR_VAL SOCKET_ERROR
#define INVALID_SOCKET_VAL INVALID_SOCKET
#define CLOSE_SOCKET closesocket
#define GET_SOCKET_ERROR WSAGetLastError()
#define SOCKET_EWOULDBLOCK WSAEWOULDBLOCK
#define SOCKET_ECONNRESET WSAECONNRESET
#define SOCKET_ECONNABORTED WSAECONNABORTED
#define SOCKET_ENOTCONN WSAENOTCONN
#define SOCKET_ENETDOWN WSAENETDOWN
#define SOCKET_ETIMEDOUT WSAETIMEDOUT
#define SOCKET_EINTR WSAEINTR
#define SOCKET_ESHUTDOWN WSAESHUTDOWN
#define SOCKET_ENETRESET WSAENETRESET

// Windows连接相关错误码
#define SOCKET_ECONNREFUSED WSAECONNREFUSED
#define SOCKET_EHOSTUNREACH WSAEHOSTUNREACH
#define SOCKET_ENETUNREACH WSAENETUNREACH
#define SOCKET_EADDRINUSE WSAEADDRINUSE
#define SOCKET_EINPROGRESS WSAEINPROGRESS
#define SOCKET_EACCES WSAEACCES
#define SOCKET_EISCONN WSAEISCONN
#define SOCKET_EFAULT WSAEFAULT

// Windows专用函数声明
#define SOCKET_NONBLOCK(sock)              \
    {                                      \
        u_long mode = 1;                   \
        ioctlsocket(sock, FIONBIO, &mode); \
    }
#define SOCKET_BLOCK(sock)                 \
    {                                      \
        u_long mode = 0;                   \
        ioctlsocket(sock, FIONBIO, &mode); \
    }

// Windows ICMP
typedef HANDLE IcmpHandle;
#define ICMP_CREATE_FILE() IcmpCreateFile()
#define ICMP_CLOSE_HANDLE(h) IcmpCloseHandle(h)
#define ICMP_INVALID_HANDLE INVALID_HANDLE_VALUE
#define IP_SUCCESS 0

#else
// Linux/Unix头文件
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <string.h>
#include <signal.h>
#include <netdb.h>
#include <sys/epoll.h>

#define SOCKET_TYPE int
#define SOCKET_ERROR_VAL (-1)
#define INVALID_SOCKET_VAL (-1)
#define CLOSE_SOCKET close
#define GET_SOCKET_ERROR errno
#define SOCKET_EWOULDBLOCK EWOULDBLOCK
#define SOCKET_ECONNRESET ECONNRESET
#define SOCKET_ECONNABORTED ECONNABORTED
#define SOCKET_ENOTCONN ENOTCONN
#define SOCKET_ENETDOWN ENETDOWN
#define SOCKET_ETIMEDOUT ETIMEDOUT
#define SOCKET_EINTR EINTR
#define SOCKET_ESHUTDOWN ESHUTDOWN
#define SOCKET_ENETRESET ENETRESET

// Linux连接相关错误码
#define SOCKET_ECONNREFUSED ECONNREFUSED
#define SOCKET_EHOSTUNREACH EHOSTUNREACH
#define SOCKET_ENETUNREACH ENETUNREACH
#define SOCKET_EADDRINUSE EADDRINUSE
#define SOCKET_EINPROGRESS EINPROGRESS
#define SOCKET_EACCES EACCES
#define SOCKET_EISCONN EISCONN
#define SOCKET_EFAULT EFAULT

// Linux专用函数声明
#define SOCKET_NONBLOCK(sock)                     \
    {                                             \
        int flags = fcntl(sock, F_GETFL, 0);      \
        fcntl(sock, F_SETFL, flags | O_NONBLOCK); \
    }
#define SOCKET_BLOCK(sock)                         \
    {                                              \
        int flags = fcntl(sock, F_GETFL, 0);       \
        fcntl(sock, F_SETFL, flags & ~O_NONBLOCK); \
    }

// Linux ICMP相关定义
typedef struct icmphdr ICMPHeader;
#define ICMP_ECHO 8
#define ICMP_ECHOREPLY 0

// 自定义ICMP回复结构，模拟Windows的ICMP_ECHO_REPLY
struct ICMP_ECHO_REPLY
{
    uint32_t Address;       // 回复地址
    uint32_t Status;        // 回复状态
    uint32_t RoundTripTime; // 往返时间(ms)
    uint16_t DataSize;      // 回复数据大小
    uint16_t Reserved;      // 保留
    void *Data;             // 数据指针
    struct ip_options
    {
        uint8_t Ttl;          // TTL值
        uint8_t Tos;          // TOS值
        uint8_t Flags;        // IP标志
        uint8_t OptionsSize;  // 选项大小
        uint8_t *OptionsData; // 选项数据
    } Options;
};
typedef ICMP_ECHO_REPLY *PICMP_ECHO_REPLY;

typedef int IcmpHandle;
#define ICMP_CREATE_FILE() socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)
#define ICMP_CLOSE_HANDLE(h) close(h)
#define ICMP_INVALID_HANDLE (-1)
#define IP_SUCCESS 0

// 主机名最大长度（Linux标准）
#ifndef NI_MAXHOST
#define NI_MAXHOST 1025
#endif

#define closesocket close

#endif

// 统一的Socket类型
typedef SOCKET_TYPE UnifiedSocket;

#endif // PLATFORM_SOCKET_H