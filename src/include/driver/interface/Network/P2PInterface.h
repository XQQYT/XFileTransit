#ifndef _P2PINTERFACE_H
#define _P2PINTERFACE_H

#include "driver/interface/Network/NetworkInterface.h"

class P2PInterface : public NetworkInterface
{
public:
    enum class IceState
    {
        New = 0,          // 新连接，未开始
        Checking = 1,     // 正在检查候选对
        Connected = 2,    // 找到有效候选对，连接建立
        Completed = 3,    // 所有候选收集完成，连接优化完成
        Failed = 4,       // 连接失败
        Disconnected = 5, // 连接断开
        Closed = 6        // 连接关闭
    };

    virtual ~P2PInterface() = default;
    virtual void initialize() = 0;
    virtual void setMsgParser(std::function<void(std::string)>) = 0;
    virtual void createOffer(std::function<void(const std::string &offer)> callback) = 0;
    virtual void setRemoteDescription(const std::string &sdp,
                                      std::function<void(bool, const std::string &answer)> callback = nullptr) = 0;
    virtual void addIceCandidate(const std::string &candidate) = 0;
    virtual void setIceGenerateCb(std::function<void(const std::string &)> cb) = 0;
    virtual void setIceStatusCb(std::function<void(const IceState)> cb) = 0;
};

#endif