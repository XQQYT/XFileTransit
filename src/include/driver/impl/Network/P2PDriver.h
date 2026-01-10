#ifndef _P2PDRIVER_H
#define _P2PDRIVER_H

#include "driver/interface/Network/NetworkInterface.h"
#include "driver/interface/FileSyncEngine/FileReceiverInterface.h"
#include "driver/interface/FileSyncEngine/FileSenderInterface.h"
#include "driver/interface/Network/P2PInterface.h"
#include <rtc/peerconnection.hpp>
#include <rtc/datachannel.hpp>
#include <unordered_map>

class P2PDriver : public P2PInterface
{
public:
    P2PDriver();

    void initialize();
    void startListen(const std::string &address, const std::string &port, std::function<bool(bool)> callback) {}

    // 用户数据
    void connect(const std::string &address, const std::string &port, std::function<void(bool)> callback = nullptr) override;
    void sendMsg(const std::string &msg, std::string label = "") override;
    void recvMsg(std::function<void(std::string)> callback) override;
    void closeSocket() override;
    void resetConnection() override;

    void createOffer(std::function<void(const std::string &offer)> callback) override;
    void setRemoteDescription(const std::string &sdp,
                              std::function<void(bool, const std::string &answer)> callback = nullptr) override;
    void addIceCandidate(const std::string &candidate) override;
    void setIceGenerateCb(std::function<void(const std::string &)> cb) override;
    void setIceStatusCb(std::function<void(const IceState)> cb) override;

    void receiveDataChannel(std::shared_ptr<rtc::DataChannel>);
    void setMsgParser(std::function<void(std::string)> parser) override
    {
        msg_callback = parser;
    }

private:
    enum class Role
    {
        Default,
        Offer,
        Answer
    };
    std::shared_ptr<NetworkInterface> websocket_driver;
    std::unique_ptr<rtc::PeerConnection> peer_connection;
    std::unordered_map<std::string, std::shared_ptr<rtc::DataChannel>> label_dc_map;
    Role current_role;
    std::function<void(std::string)> msg_callback;
    rtc::PeerConnection::IceState ice_state;
};

#endif