#include "driver/impl/MsgBuilder.h"
#include <memory.h>
#include <iostream>
#include <fstream>

MsgBuilder::MsgBuilder(std::shared_ptr<SecurityInterface> instance)
    :version(0x01)
{
    security_instance = instance;
}

std::unique_ptr<MsgBuilderInterface::UserMsg> MsgBuilder::buildMsg(std::string real_msg, const uint8_t* key)
{
    Header header;

    uint16_t net_magic = htons(magic);

    memcpy(&header.magic, &net_magic, sizeof(net_magic));
    memcpy(&header.version, &version, sizeof(version));

    uint8_t* iv = nullptr;
    uint8_t* sha256 = nullptr;

    std::vector<uint8_t> vec(reinterpret_cast<const uint8_t*>(real_msg.data()),
        reinterpret_cast<const uint8_t*>(real_msg.data() + real_msg.size()));

    if (security_instance) {
        iv = security_instance->aesEncrypt(vec, key);
        //向量+内容一起做sha256
        std::vector<uint8_t> vi_encrypt(iv, iv + 16);
        vi_encrypt.insert(vi_encrypt.end(), vec.begin(), vec.end());
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
        payload_length = htonl(1 + 1 + 16 + 32 + vec.size());
    }
    else
    {
        payload_length = htonl(1 + 1 + vec.size());
    }
    std::vector<uint8_t> msg(sizeof(Header) + payload_length);

    memcpy(&header.length, &payload_length, sizeof(payload_length));

    size_t offset = 0;
    //0-6字节，消息头
    memcpy(msg.data() + offset, &header, sizeof(Header)); offset += sizeof(Header);
    //7字节表示是否为二进制消息
    *(msg.data() + offset) = 0x00; offset += 1;
    //8字节表示是否加密
    *(msg.data() + offset) = static_cast<uint8_t>(security_instance.operator bool());offset += 1;
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
    memcpy(msg.data() + offset, vec.data(), vec.size()); offset += vec.size();
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

std::unique_ptr<MsgBuilderInterface::UserMsg> MsgBuilder::buildFile(MsgBuilderInterface::MessageType type, std::string username, std::string path, uint8_t* key)
{
    constexpr size_t USERNAME_SIZE = 15;
    if (username.size() > USERNAME_SIZE) {
        username.resize(USERNAME_SIZE);
    }

    Header header;

    uint16_t net_magic = htons(magic);
    uint32_t length_place_holder = 0x0;

    memcpy(&header.magic, &net_magic, sizeof(net_magic));
    memcpy(&header.version, &version, sizeof(version));
    memcpy(&header.length, &length_place_holder, sizeof(length_place_holder));

    uint8_t* iv = nullptr;
    uint8_t* sha256 = nullptr;

    std::unique_ptr<std::vector<uint8_t>> full_content = std::make_unique<std::vector<uint8_t>>(getFileSize(path) + 2 + USERNAME_SIZE);
    uint16_t head_offset = 0;
    uint16_t type_net = htons(static_cast<uint16_t>(type));
    memcpy(full_content->data() + head_offset, &type_net, 2);
    head_offset += 2;
    memcpy(full_content->data() + head_offset, username.data(), username.size());
    if (username.size() < USERNAME_SIZE) {
        memset(full_content->data() + head_offset + username.size(), 0, USERNAME_SIZE - username.size());
    }
    head_offset += USERNAME_SIZE;

    auto file_all_content = readFileToBytes(path);

    memcpy(full_content->data() + head_offset, file_all_content->data(), file_all_content->size());

    if (security_instance) {
        iv = security_instance->aesEncrypt(*full_content, key);
        std::vector<uint8_t> vi_encrypt(iv, iv + 16);
        vi_encrypt.insert(vi_encrypt.end(), full_content->begin(), full_content->end());
        sha256 = security_instance->sha256(vi_encrypt.data(), vi_encrypt.size());
    }
    else {
        iv = new uint8_t[16]();
        sha256 = new uint8_t[32]();
    }

    // 构造最终消息
    std::vector<uint8_t> msg(sizeof(Header) + 1 + 16 + full_content->size() + 32);

    uint32_t payload_length = htonl(1 + 16 + 32 + full_content->size());
    memcpy(&header.length, &payload_length, sizeof(payload_length));

    size_t offset = 0;
    memcpy(msg.data() + offset, &header, sizeof(Header)); offset += sizeof(Header);
    *(msg.data() + offset) = 0x01; offset += 1;
    memcpy(msg.data() + offset, iv, 16); offset += 16;
    memcpy(msg.data() + offset, full_content->data(), full_content->size()); offset += full_content->size();
    memcpy(msg.data() + offset, sha256, 32);

    auto user_msg = std::make_unique<MsgBuilderInterface::UserMsg>();
    user_msg->iv = iv;
    user_msg->sha256 = sha256;
    user_msg->msg = std::make_unique<std::vector<uint8_t>>(std::move(msg));

    return user_msg;
}