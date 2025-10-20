#ifndef _OPENSSLDRIVER_H
#define _OPENSSLDRIVER_H

#include "driver/interface/SecurityInterface.h"
#include <openssl/ssl.h>
#include <openssl/err.h>

class OpensslDriver : public SecurityInterface
{
public:
    OpensslDriver();
    ~OpensslDriver();

    SecurityInterface::TlsInfo getAesKey(SOCKET socket) override;
    uint8_t* aesEncrypt(std::vector<uint8_t>& data, const uint8_t* key) override;
    uint8_t* sha256(uint8_t* str, size_t length) override;
    bool verifyAndDecrypt(const std::vector<uint8_t>& encrypted_data,
        const uint8_t* key,
        const std::vector<uint8_t>& iv,
        std::vector<uint8_t>& out_plaintext,
        std::vector<uint8_t>& sha256) override;
    void dealTlsRequest(SOCKET socket, std::function<void(bool, TlsInfo)> callback) override;
    bool generateAndLoadTempCertificate();
private:
    SSL_CTX* client_ctx;  // 客户端上下文
    SSL_CTX* server_ctx;  // 服务器上下文

    bool initializeSSL();
};

#endif //_OPENSSLDRIVER_H