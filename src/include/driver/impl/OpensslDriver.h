#ifndef _OPENSSLDRIVER_H
#define _OPENSSLDRIVER_H

#include "driver/interface/SecurityInterface.h"
#include <openssl/ssl.h>
#include <openssl/err.h>

class OpensslDriver : public SecurityInterface
{
public:
    OpensslDriver();
    ~OpensslDriver() {}
    SecurityInterface::TlsInfo getAesKey(SOCKET socket) override;
    uint8_t* aesEncrypt(std::vector<uint8_t>& data, const uint8_t* key) override;
    uint8_t* sha256(uint8_t* str, size_t length) override;
    bool verifyAndDecrypt(const std::vector<uint8_t>& encrypted_data,
        const uint8_t* key,
        const std::vector<uint8_t>& iv,
        std::vector<uint8_t>& out_plaintext,
        std::vector<uint8_t>& sha256) override;
private:
    SSL_CTX* ctx;
};

#endif //_OPENSSLDRIVER_H