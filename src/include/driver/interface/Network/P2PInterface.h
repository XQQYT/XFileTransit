#ifndef _P2PINTERFACE_H
#define _P2PINTERFACE_H

#include "driver/interface/Network/NetworkInterface.h"

class P2PInterface : public NetworkInterface
{
public:
    virtual ~P2PInterface() = default;
    virtual void initialize() = 0;
    virtual void createOffer(std::function<void(const std::string &offer)> callback) = 0;
    virtual void setRemoteDescription(const std::string &sdp,
                                      std::function<void(bool)> callback = nullptr) = 0;
    virtual void addIceCandidate(const std::string &candidate) = 0;

    // P2P状态
    enum class P2PState
    {
        Idle,
        CreatingOffer,
        WaitingForAnswer,
        ExchangingCandidates,
        Connected,
        Failed
    };
    virtual P2PState getP2PState() const = 0;
};

#endif