#ifndef _SECURITYINTERFACE_H
#define _SECURITYINTERFACE_H

#include <vector>
#include <stdint.h>
#include <memory> 
#include <functional>
#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <errno.h>
    #include <fcntl.h>
    #define close_socket close
    #define SOCKET_ERROR_TIMEOUT ETIMEDOUT
    typedef int SOCKET;
    #define INVALID_SOCKET (-1)
    #define SOCKET_ERROR (-1)
#endif

class SecurityInterface
{
public:
    struct TlsInfo
    {
        std::shared_ptr<uint8_t[]> key;
    };
public:
    virtual ~SecurityInterface() = default;
    virtual TlsInfo getAesKey(SOCKET socket) = 0;
    virtual uint8_t* aesEncrypt(std::vector<uint8_t>& data, const uint8_t* key) = 0;
    virtual uint8_t* sha256(uint8_t* str, size_t length) = 0;
    virtual bool verifyAndDecrypt(const std::vector<uint8_t>& encrypted_data,
        const uint8_t* key,
        const std::vector<uint8_t>& iv,
        std::vector<uint8_t>& out_plaintext,
        std::vector<uint8_t>& sha256) = 0;
    virtual void dealTlsRequest(SOCKET socket, std::function<void(bool, TlsInfo)> callback) = 0;
    const TlsInfo getTlsInfo() { return tls_info; }
    void setTlsInfo(const TlsInfo& info) { tls_info = info; }
protected:
    TlsInfo tls_info;
};


#endif //_SECURITYINTERFACE_H