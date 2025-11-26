#include "driver/impl/MsgBuilder.h"
#include <memory.h>
#include <iostream>
#include <fstream>

MsgBuilder::MsgBuilder(std::shared_ptr<SecurityInterface> instance)
    :version(0x01)
{
    security_instance = instance;
}

std::unique_ptr<MsgBuilderInterface::UserMsg> MsgBuilder::buildMsg(std::string payload) 
{
    return build(std::vector<uint8_t>(reinterpret_cast<const uint8_t*>(payload.data()), reinterpret_cast<const uint8_t*>(payload.data()) + payload.size()));
}

std::unique_ptr<MsgBuilderInterface::UserMsg> MsgBuilder::buildMsg(std::vector<uint8_t> payload)
{
    return build(std::move(payload));
}

std::unique_ptr<MsgBuilderInterface::UserMsg> MsgBuilder::build(std::vector<uint8_t> real_msg)
{
    //构造Header
    Header header;

    uint16_t net_magic = htons(magic);

    memcpy(&header.magic, &net_magic, sizeof(net_magic));
    memcpy(&header.version, &version, sizeof(version));

    uint8_t flag = 0x0;
    if (security_instance.operator bool())
    {
        flag |= static_cast<uint8_t>(Flag::IS_ENCRYPT);
    }
    memcpy(&header.flag, &flag, sizeof(flag));

    uint8_t* iv = nullptr;
    uint8_t* sha256 = nullptr;

    if (security_instance) {
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
    if (security_instance.operator bool())
    {
        payload_length = 16 + 32 + real_msg.size();
    }
    else
    {
        payload_length = real_msg.size();
    }
    std::vector<uint8_t> msg(sizeof(Header) + payload_length);

    //补上Header中的payload length字段
    payload_length = htonl(payload_length);
    memcpy(&header.length, &payload_length, sizeof(payload_length));

    size_t offset = 0;
    //0-7字节，消息头
    memcpy(msg.data() + offset, &header, sizeof(Header)); offset += sizeof(Header);

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
    auto user_msg = std::make_unique<MsgBuilderInterface::UserMsg>();
    user_msg->iv = iv;
    user_msg->sha256 = sha256;
    user_msg->msg = std::make_unique<std::vector<uint8_t>>(std::move(msg));

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