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
    SSL_CTX_set_verify(client_ctx, SSL_VERIFY_NONE, nullptr);
    SSL_CTX_set_options(client_ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
    std::cout << "SSL client context created successfully" << std::endl;

    // 2. 初始化服务器上下文
    server_ctx = SSL_CTX_new(TLS_server_method());
    if (!server_ctx) {
        std::cerr << "Unable to create SSL server context" << std::endl;
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(client_ctx);
        client_ctx = nullptr;
        return false;
    }
    SSL_CTX_set_verify(server_ctx, SSL_VERIFY_NONE, nullptr);
    SSL_CTX_set_options(server_ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
    std::cout << "SSL server context created successfully" << std::endl;

    std::cout << "OpenSSL dual mode initialization completed" << std::endl;
    return true;
}

void OpensslDriver::safeSSLShutdown(SSL* ssl, bool handshakeCompleted)
{
    if (!ssl) return;

    if (handshakeCompleted) {
        // 只有握手成功后才尝试优雅关闭
        SSL_shutdown(ssl);
    }
    SSL_free(ssl);
}

SecurityInterface::TlsInfo OpensslDriver::getAesKey(SOCKET socket)
{
    constexpr const uint32_t KEYLENGTH = 32;

    if (!client_ctx) {
        throw std::runtime_error("SSL client context not initialized");
    }

    SSL* ssl = SSL_new(client_ctx);  // 使用客户端上下文
    if (!ssl) {
        throw std::runtime_error("SSL_new failed for client");
    }

    SSL_set_fd(ssl, socket);

    bool handshakeCompleted = false;

    try {
        std::cout << "=== Client: Starting TLS connection ===" << std::endl;

        int ret = SSL_connect(ssl);
        std::cout << "SSL_connect returned: " << ret << std::endl;

        if (ret <= 0) {
            int ssl_error = SSL_get_error(ssl, ret);
            std::cerr << "SSL client connection failed with error: " << ssl_error << std::endl;
            ERR_print_errors_fp(stderr);
            throw std::runtime_error("SSL_connect failed");
        }

        handshakeCompleted = true;
        std::cout << "Client TLS handshake completed!" << std::endl;
        std::cout << "Protocol: " << SSL_get_version(ssl) << ", Cipher: " << SSL_get_cipher(ssl) << std::endl;

        // 读取服务器发送的密钥
        std::unique_ptr<uint8_t[]> key(new uint8_t[KEYLENGTH]);
        memset(key.get(), 0, KEYLENGTH);

        int total_read = 0;
        while (total_read < KEYLENGTH) {
            int n = SSL_read(ssl, key.get() + total_read, KEYLENGTH - total_read);
            if (n <= 0) {
                int ssl_error = SSL_get_error(ssl, n);
                if (ssl_error == SSL_ERROR_ZERO_RETURN) {
                    break; // 正常关闭
                }
                throw std::runtime_error("SSL_read failed");
            }
            total_read += n;
        }

        if (total_read < KEYLENGTH) {
            throw std::runtime_error("Incomplete key received");
        }

        std::cout << "Successfully received key from server: " << total_read << " bytes" << std::endl;

        // 安全关闭连接
        safeSSLShutdown(ssl, handshakeCompleted);
        closesocket(socket);

        std::cout << "TLS client connection closed" << std::endl;
        return SecurityInterface::TlsInfo{ key.release() };

    }
    catch (...) {
        safeSSLShutdown(ssl, handshakeCompleted);
        closesocket(socket);
        throw;
    }
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

    std::unique_ptr<uint8_t[]> key(new uint8_t[32]);
    bool handshakeCompleted = false;

    try {
        std::cout << "=== Server: Starting TLS handshake ===" << std::endl;

        int ret = SSL_accept(ssl);
        std::cout << "SSL_accept returned: " << ret << std::endl;

        if (ret <= 0) {
            int ssl_error = SSL_get_error(ssl, ret);
            std::cerr << "SSL server handshake failed with error: " << ssl_error << std::endl;
            ERR_print_errors_fp(stderr);
            throw std::runtime_error("SSL_accept failed");
        }

        handshakeCompleted = true;
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
        safeSSLShutdown(ssl, handshakeCompleted);
        closesocket(socket);

        callback(true, { key.release() });

    }
    catch (const std::exception& e) {
        std::cerr << "Exception in dealTlsRequest: " << e.what() << std::endl;
        safeSSLShutdown(ssl, handshakeCompleted);
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