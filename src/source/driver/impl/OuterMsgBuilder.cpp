#include "driver/impl/OuterMsgBuilder.h"
#include <memory.h>
#include <iostream>
#include <fstream>

OuterMsgBuilder::OuterMsgBuilder(std::shared_ptr<SecurityInterface> instance)
    :version(0x01)
{
    security_instance = instance;
}

std::unique_ptr<NetworkInterface::UserMsg> OuterMsgBuilder::buildMsg(std::string payload, NetworkInterface::Flag flag)
{
    return build(
        std::vector<uint8_t>(reinterpret_cast<const uint8_t*>(payload.data()), reinterpret_cast<const uint8_t*>(payload.data()) + payload.size()),
        flag);
}

std::unique_ptr<NetworkInterface::UserMsg> OuterMsgBuilder::buildMsg(std::vector<uint8_t> payload, NetworkInterface::Flag flag)
{
    return build(std::move(payload), flag);
}

std::unique_ptr<NetworkInterface::UserMsg> OuterMsgBuilder::build(std::vector<uint8_t> real_msg, NetworkInterface::Flag flag)
{
    //构造Header
    NetworkInterface::Header header;

    uint16_t net_magic = htons(NetworkInterface::magic);

    memcpy(&header.magic, &net_magic, sizeof(net_magic));
    memcpy(&header.version, &version, sizeof(version));

    uint8_t msg_flag = static_cast<uint8_t>(flag);

    memcpy(&header.flag, &msg_flag, sizeof(msg_flag));

    uint8_t* iv = nullptr;
    uint8_t* sha256 = nullptr;

    bool encrypt = security_instance && (flag & NetworkInterface::Flag::IS_ENCRYPT);
    if (encrypt) {
        iv = security_instance->aesEncrypt(real_msg, security_instance->getTlsInfo().key.get());
        //向量+内容一起做sha256
        std::vector<uint8_t> vi_encrypt(iv, iv + 16);
        vi_encrypt.insert(vi_encrypt.end(), real_msg.begin(), real_msg.end());
        sha256 = security_instance->sha256(vi_encrypt.data(), vi_encrypt.size());
    }
    else {
        iv = nullptr;
        sha256 = nullptr;
    }

    //计算载荷长度
    uint32_t payload_length;
    if (encrypt)
    {
        payload_length = 16 + 32 + real_msg.size();
    }
    else
    {
        payload_length = real_msg.size();
    }
    std::vector<uint8_t> msg(sizeof(NetworkInterface::Header) + payload_length);

    //补上Header中的payload length字段
    payload_length = htonl(payload_length);
    memcpy(&header.length, &payload_length, sizeof(payload_length));

    size_t offset = 0;
    //0-7字节，消息头
    memcpy(msg.data() + offset, &header, sizeof(NetworkInterface::Header)); offset += sizeof(NetworkInterface::Header);

    //加密则写入iv(16字节)sha256(32字节)，不加密则不写入
    if (iv)
    {
        memcpy(msg.data() + offset, iv, 16);
        offset += 16;
    }
    if (sha256)
    {
        memcpy(msg.data() + offset, sha256, 32);
        offset += 32;
    }
    //拷贝加密后的数据
    memcpy(msg.data() + offset, real_msg.data(), real_msg.size()); offset += real_msg.size();
    auto user_msg = std::make_unique<NetworkInterface::UserMsg>();
    user_msg->iv.assign(iv, iv + 16);
    user_msg->sha256.assign(sha256, sha256 + 32);
    user_msg->data = std::move(msg);

    return user_msg;
}

std::unique_ptr<std::vector<uint8_t>> readFileToBytes(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) throw std::runtime_error("Failed to open file");

    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        throw std::runtime_error("Failed to read file");
    }

    return std::make_unique<std::vector<uint8_t>>(std::move(buffer));
}

std::streamsize getFileSize(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate); // 打开文件，光标定位到末尾
    if (!file) {
        throw std::runtime_error("Failed to open file");
    }
    return file.tellg(); // 返回当前位置，也就是文件大小
}