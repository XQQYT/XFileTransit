#include "driver/impl/OuterMsgParser.h"

std::unique_ptr<OuterMsgParserInterface::ParsedMsg> OuterMsgParser::parse(std::vector<uint8_t>&& msg, const uint32_t length, const uint8_t flag)
{
    OuterMsgParserInterface::ParsedMsg result;

    size_t offset = 0;

    // 检查标志位
    bool is_encrypt = static_cast<bool>((flag) & static_cast<uint8_t>(OuterMsgBuilderInterface::Flag::IS_ENCRYPT));

    if (is_encrypt)
    {
        // 解析 IV（16字节）
        result.iv.assign(msg.data() + offset, msg.data() + offset + 16);
        offset += 16;

        // 解析 SHA256（32字节）
        result.sha256.assign(msg.data() + offset, msg.data() + offset + 32);
        offset += 32;
    }

    // 解析密文
    size_t cipher_len = length;
    if (is_encrypt)
    {
        cipher_len = length - 16 - 32;
    }

    result.data.assign(msg.data() + offset, msg.data() + offset + cipher_len);
    offset += cipher_len;

    return std::make_unique<OuterMsgParserInterface::ParsedMsg>(std::move(result));
}