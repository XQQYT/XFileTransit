#include "driver/impl/OpensslDriver.h"
#include <openssl/aes.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <openssl/applink.c>
#include <cstring> 
#include <string>
#include <iostream>
#include <memory>

OpensslDriver::OpensslDriver() : client_ctx(nullptr), server_ctx(nullptr)
{
    if (!initializeSSL()) {
        std::cerr << "Failed to initialize OpenSSL" << std::endl;
    }
}

OpensslDriver::~OpensslDriver()
{
    if (client_ctx) {
        SSL_CTX_free(client_ctx);
    }
    if (server_ctx) {
        SSL_CTX_free(server_ctx);
    }
}

bool OpensslDriver::initializeSSL()
{
    std::cout << "Initializing OpenSSL in dual mode..." << std::endl;

    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    // 1. 初始化客户端上下文
    client_ctx = SSL_CTX_new(TLS_client_method());
    if (!client_ctx) {
        std::cerr << "Unable to create SSL client context" << std::endl;
        ERR_print_errors_fp(stderr);
        return false;
    }
    SSL_CTX_set_min_proto_version(client_ctx, TLS1_2_VERSION);
    std::cout << "SSL client context created successfully" << std::endl;

    server_ctx = SSL_CTX_new(TLS_server_method());
    if (!server_ctx) {
        std::cerr << "Unable to create SSL server context" << std::endl;
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(client_ctx);
        client_ctx = nullptr;
        return false;
    }
    SSL_CTX_set_min_proto_version(server_ctx, TLS1_2_VERSION);

    if (!generateAndLoadTempCertificate()) {
        std::cerr << "Failed to generate temporary certificate" << std::endl;
        SSL_CTX_free(server_ctx);
        SSL_CTX_free(client_ctx);
        server_ctx = nullptr;
        client_ctx = nullptr;
        return false;
    }

    std::cout << "SSL server context created with certificate" << std::endl;
    std::cout << "OpenSSL dual mode initialization completed" << std::endl;
    return true;
}

bool OpensslDriver::generateAndLoadTempCertificate()
{
    EVP_PKEY* pkey = EVP_PKEY_new();
    RSA* rsa = RSA_new();
    BIGNUM* bn = BN_new();

    // 生成RSA密钥
    BN_set_word(bn, RSA_F4);
    RSA_generate_key_ex(rsa, 2048, bn, nullptr);
    EVP_PKEY_assign_RSA(pkey, rsa);

    // 创建证书
    X509* x509 = X509_new();
    X509_set_version(x509, 2);
    ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);
    X509_gmtime_adj(X509_get_notBefore(x509), 0);
    X509_gmtime_adj(X509_get_notAfter(x509), 365 * 24 * 60 * 60L);

    X509_set_pubkey(x509, pkey);

    X509_NAME* name = X509_get_subject_name(x509);
    X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC, (unsigned char*)"US", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "O", MBSTRING_ASC, (unsigned char*)"Test Org", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char*)"localhost", -1, -1, 0);

    X509_set_issuer_name(x509, name);
    X509_sign(x509, pkey, EVP_sha256());

    // 加载到SSL上下文
    if (SSL_CTX_use_certificate(server_ctx, x509) <= 0) {
        std::cerr << "Failed to load certificate" << std::endl;
        ERR_print_errors_fp(stderr);
        X509_free(x509);
        EVP_PKEY_free(pkey);
        BN_free(bn);
        return false;
    }

    if (SSL_CTX_use_PrivateKey(server_ctx, pkey) <= 0) {
        std::cerr << "Failed to load private key" << std::endl;
        ERR_print_errors_fp(stderr);
        X509_free(x509);
        EVP_PKEY_free(pkey);
        BN_free(bn);
        return false;
    }

    X509_free(x509);
    EVP_PKEY_free(pkey);
    BN_free(bn);

    std::cout << "Temporary self-signed certificate generated and loaded" << std::endl;
    return true;
}

SecurityInterface::TlsInfo OpensslDriver::getAesKey(SOCKET socket)
{
    constexpr const uint32_t KEYLENGTH = 32;

    std::cout << "=== Starting TLS handshake ===" << std::endl;

    // 创建SSL对象
    SSL* ssl = SSL_new(client_ctx);
    if (!ssl) {
        std::cerr << "Failed to create SSL object" << std::endl;
        ERR_print_errors_fp(stderr);
        throw std::runtime_error("SSL_new failed");
    }

    // 将SSL与socket关联
    if (SSL_set_fd(ssl, socket) != 1) {
        std::cerr << "Failed to set SSL file descriptor" << std::endl;
        SSL_free(ssl);
        throw std::runtime_error("SSL_set_fd failed");
    }

    std::cout << "Starting SSL_connect..." << std::endl;

    // 执行TLS握手
    int ret = SSL_connect(ssl);
    std::cout << "SSL_connect returned: " << ret << std::endl;

    if (ret <= 0) {
        int ssl_error = SSL_get_error(ssl, ret);
        std::cerr << "SSL connection failed with error: " << ssl_error << std::endl;

        switch (ssl_error) {
        case SSL_ERROR_NONE:
            std::cerr << "SSL_ERROR_NONE - No error" << std::endl;
            break;
        case SSL_ERROR_ZERO_RETURN:
            std::cerr << "SSL_ERROR_ZERO_RETURN - TLS connection closed" << std::endl;
            break;
        case SSL_ERROR_WANT_READ:
        case SSL_ERROR_WANT_WRITE:
            std::cerr << "SSL_ERROR_WANT_READ/WRITE - Operation would block" << std::endl;
            break;
        case SSL_ERROR_SYSCALL:
            std::cerr << "SSL_ERROR_SYSCALL - System call error: " << ERR_get_error() << std::endl;
            break;
        case SSL_ERROR_SSL:
            std::cerr << "SSL_ERROR_SSL - SSL protocol error" << std::endl;
            break;
        default:
            std::cerr << "Unknown SSL error" << std::endl;
        }

        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        throw std::runtime_error("SSL_connect failed");
    }

    std::cout << "TLS handshake completed successfully!" << std::endl;
    std::cout << "Protocol: " << SSL_get_version(ssl) << std::endl;
    std::cout << "Cipher: " << SSL_get_cipher(ssl) << std::endl;

    // 读取服务器发送的密钥
    std::shared_ptr<uint8_t[]> key(new uint8_t[KEYLENGTH]);
    int total_read = 0;

    while (total_read < KEYLENGTH) {
        int n = SSL_read(ssl, key.get() + total_read, KEYLENGTH - total_read);
        if (n > 0) {
            total_read += n;
            std::cout << "Read " << n << " bytes, total: " << total_read << "/" << KEYLENGTH << std::endl;
        }
        else if (n == 0) {
            std::cerr << "SSL connection closed by peer" << std::endl;
            break;
        }
        else {
            int ssl_error = SSL_get_error(ssl, n);
            if (ssl_error == SSL_ERROR_WANT_READ || ssl_error == SSL_ERROR_WANT_WRITE) {
                continue; // 重试
            }
            std::cerr << "SSL_read failed with error: " << ssl_error << std::endl;
            break;
        }
    }

    if (total_read < KEYLENGTH) {
        std::cerr << "Failed to receive complete key. Received: " << total_read << "/" << KEYLENGTH << " bytes" << std::endl;
        SSL_shutdown(ssl);
        SSL_free(ssl);
        throw std::runtime_error("Incomplete key received");
    }

    std::cout << "Successfully received encryption key from server" << std::endl;

    // 清理SSL连接
    SSL_shutdown(ssl);
    SSL_free(ssl);

    return SecurityInterface::TlsInfo{ key };
}

void OpensslDriver::dealTlsRequest(SOCKET socket, std::function<void(bool, TlsInfo)> callback)
{
    if (!server_ctx) {
        std::cerr << "SSL server context not initialized" << std::endl;
        callback(false, { nullptr });
        return;
    }

    SSL* ssl = SSL_new(server_ctx);  // 使用服务器上下文
    if (!ssl) {
        std::cerr << "SSL_new failed for server" << std::endl;
        callback(false, { nullptr });
        return;
    }

    SSL_set_fd(ssl, socket);


    try {
        std::cout << "=== Server: Starting TLS handshake ===" << std::endl;
        int ret = SSL_accept(ssl);
        std::cout << "SSL_accept returned: " << ret << std::endl;
        std::shared_ptr<uint8_t[]> key(new uint8_t[32]);

        if (ret <= 0) {
            int ssl_error = SSL_get_error(ssl, ret);
            std::cerr << "SSL server handshake failed with error: " << ssl_error << std::endl;
            ERR_print_errors_fp(stderr);
            throw std::runtime_error("SSL_accept failed");
        }

        std::cout << "Server TLS handshake completed!" << std::endl;
        std::cout << "Protocol: " << SSL_get_version(ssl) << ", Cipher: " << SSL_get_cipher(ssl) << std::endl;

        // 生成随机密钥
        if (RAND_bytes(key.get(), 32) != 1) {
            throw std::runtime_error("RAND_bytes failed");
        }

        // 发送密钥给客户端
        int bytes_sent = SSL_write(ssl, key.get(), 32);
        if (bytes_sent <= 0) {
            int ssl_error = SSL_get_error(ssl, bytes_sent);
            std::cerr << "Failed to send key to client, SSL error: " << ssl_error << std::endl;
            throw std::runtime_error("SSL_write failed");
        }

        std::cout << "Successfully sent key to client: " << bytes_sent << " bytes" << std::endl;

        // 安全关闭连接
        SSL_shutdown(ssl);
        SSL_free(ssl);
        closesocket(socket);

        callback(true, { key });

    }
    catch (const std::exception& e) {
        std::cerr << "Exception in dealTlsRequest: " << e.what() << std::endl;
        SSL_shutdown(ssl);
        SSL_free(ssl);
        closesocket(socket);
        callback(false, { nullptr });
    }
}

// 以下函数保持不变（CRC32、AES加密、SHA256等）
// 生成 CRC32 查找表
void generateCRC32Table(uint32_t* table) {
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t c = i;
        for (int j = 0; j < 8; j++) {
            if (c & 1) {
                c = 0xEDB88320 ^ (c >> 1);
            }
            else {
                c = c >> 1;
            }
        }
        table[i] = c;
    }
}

// 计算 CRC32 值
uint32_t calculateCRC32(const uint8_t* data, size_t length) {
    static uint32_t table[256];
    static bool tableGenerated = false;

    if (!tableGenerated) {
        generateCRC32Table(table);
        tableGenerated = true;
    }

    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; i++) {
        crc = table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFF;
}

uint8_t* OpensslDriver::aesEncrypt(std::vector<uint8_t>& data, const uint8_t* key)
{
    // 1. 计算 CRC32 并附加到数据末尾
    uint32_t crc = calculateCRC32(data.data(), data.size());

    // 将 CRC32 以小端字节序附加到数据末尾
    data.push_back(static_cast<uint8_t>(crc & 0xFF));
    data.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>((crc >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((crc >> 24) & 0xFF));

    // 2. 添加 PKCS#7 填充
    size_t blockSize = AES_BLOCK_SIZE;
    size_t paddingLength = blockSize - (data.size() % blockSize);
    data.insert(data.end(), paddingLength, static_cast<uint8_t>(paddingLength));

    size_t paddedLen = data.size();

    // 3. 生成随机 IV
    uint8_t* iv = new uint8_t[blockSize];
    if (RAND_bytes(iv, blockSize) != 1) {
        std::cerr << "Failed to generate IV" << std::endl;
        delete[] iv;
        return nullptr;
    }

    AES_KEY aesKey;
    if (AES_set_encrypt_key(key, 256, &aesKey) < 0) {
        std::cerr << "Failed to set AES key" << std::endl;
        delete[] iv;
        return nullptr;
    }

    std::vector<uint8_t> encrypted(paddedLen);
    uint8_t iv_copy[AES_BLOCK_SIZE];
    memcpy(iv_copy, iv, AES_BLOCK_SIZE);

    AES_cbc_encrypt(data.data(), encrypted.data(), paddedLen, &aesKey, iv_copy, AES_ENCRYPT);

    data = std::move(encrypted);

    return iv;
}

uint8_t* OpensslDriver::sha256(uint8_t* str, size_t length)
{
    uint8_t* digest = new uint8_t[SHA256_DIGEST_LENGTH];

    if (!SHA256(str, length, digest)) {
        std::cerr << "SHA256 calculation failed" << std::endl;
        delete[] digest;
        return nullptr;
    }

    return digest;
}

bool verify_sha256(const std::vector<uint8_t>& data, const std::vector<uint8_t>& expected_hash) {
    if (expected_hash.size() != SHA256_DIGEST_LENGTH) {
        std::cerr << "Invalid hash length." << std::endl;
        return false;
    }

    // 计算实际的 SHA-256 哈希
    uint8_t hash[SHA256_DIGEST_LENGTH];
    SHA256(data.data(), data.size(), hash);

    // 比较实际哈希和期望哈希
    return std::memcmp(hash, expected_hash.data(), SHA256_DIGEST_LENGTH) == 0;
}

bool OpensslDriver::verifyAndDecrypt(const std::vector<uint8_t>& encrypted_data,
    const uint8_t* key,
    const std::vector<uint8_t>& iv,
    std::vector<uint8_t>& out_plaintext,
    std::vector<uint8_t>& sha256_hash) {
    if (iv.size() != AES_BLOCK_SIZE) {
        std::cerr << "IV size incorrect" << std::endl;
        return false;
    }

    std::vector<uint8_t> iv_encrypted(iv.begin(), iv.end());
    iv_encrypted.insert(iv_encrypted.end(), encrypted_data.begin(), encrypted_data.end());

    if (!verify_sha256(iv_encrypted, sha256_hash))
    {
        std::cout << "sha256校验失败" << std::endl;
        return false;
    }

    AES_KEY aesKey;
    if (AES_set_decrypt_key(key, 256, &aesKey) < 0) {
        std::cerr << "Failed to set AES decryption key" << std::endl;
        return false;
    }

    out_plaintext.resize(encrypted_data.size());
    uint8_t iv_copy[AES_BLOCK_SIZE];
    std::memcpy(iv_copy, iv.data(), AES_BLOCK_SIZE);

    AES_cbc_encrypt(encrypted_data.data(),
        out_plaintext.data(),
        encrypted_data.size(),
        &aesKey,
        iv_copy,
        AES_DECRYPT);

    // 1. 移除 PKCS#7 填充
    if (!out_plaintext.empty()) {
        uint8_t padding_size = out_plaintext.back();

        if (padding_size == 0 || padding_size > AES_BLOCK_SIZE) {
            std::cerr << "Invalid padding size" << std::endl;
            return false;
        }

        bool padding_valid = true;
        size_t start = out_plaintext.size() - padding_size;
        for (size_t i = start; i < out_plaintext.size(); ++i) {
            if (out_plaintext[i] != padding_size) {
                padding_valid = false;
                break;
            }
        }

        if (!padding_valid) {
            std::cerr << "Invalid padding content" << std::endl;
            return false;
        }

        out_plaintext.resize(start);
    }

    // 2. 移除 CRC32（4字节）
    if (out_plaintext.size() < sizeof(uint32_t)) {
        std::cerr << "Data too short to contain CRC32" << std::endl;
        return false;
    }
    out_plaintext.resize(out_plaintext.size() - sizeof(uint32_t));

    return true;
}