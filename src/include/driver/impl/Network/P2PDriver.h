#ifndef _P2PDRIVER_H
#define _P2PDRIVER_H

#include "driver/interface/FileSyncEngine/FileParserInterface.h"
#include "driver/interface/Network/P2PInterface.h"
#include "driver/impl/OuterMsgParser.h"
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
    void sendMsg(const std::vector<uint8_t> &msg, std::string label = "") override;
    void recvMsg(std::function<void(std::string)> callback) override;
    void closeSocket() override;
    void resetConnection() override;

    void createOffer(std::function<void(const std::string &offer)> callback) override;
    void setRemoteDescription(const std::string &sdp,
                              std::function<void(bool, const std::string &answer)> callback = nullptr) override;
    void addIceCandidate(const std::string &candidate) override;

    void setIceGenerateCb(std::function<void(const std::string &)> cb) override
    {
        ice_generate_cb = cb;
    }
    void setIceStatusCb(std::function<void(const IceState)> cb) override
    {
        ice_status_cb = cb;
    }

    void receiveDataChannel(std::shared_ptr<rtc::DataChannel>);
    void setMsgParser(std::function<void(std::string)> parser) override
    {
        msg_callback = parser;
    }
    void parseRecvMsg(rtc::message_variant msg);

    std::string getOneReceiveDc() override
    {
        if (cur_sender_index >= sender_label_list.size())
        {
            return "";
        }
        return sender_label_list[cur_sender_index++];
    }

private:
    enum class Role
    {
        Default,
        Offer,
        Answer
    };
    int cur_sender_index{0};
    std::shared_ptr<NetworkInterface> websocket_driver;
    std::unique_ptr<rtc::PeerConnection> peer_connection;
    std::unique_ptr<OuterMsgParser> outer_parser;
    std::unordered_map<std::string, std::shared_ptr<rtc::DataChannel>> label_dc_map;
    Role current_role;
    std::function<void(std::string)> msg_callback;
    rtc::PeerConnection::IceState ice_state;

    std::function<void(const std::string &)> ice_generate_cb;
    std::function<void(const IceState)> ice_status_cb;

    std::vector<std::unique_ptr<FileParserInterface>> file_parsers;
    std::vector<std::string> sender_label_list;
};

#endif