#include "driver/impl/Network/P2PDriver.h"
#include "driver/impl/Network/WebSocket.h"
#include "common/DebugOutputer.h"
#include <rtc/configuration.hpp>

P2PDriver::P2PDriver()
    : websocket_driver(WebSocket::create())
{
    current_role = Role::Default;
}

void P2PDriver::initialize()
{
    rtc::Configuration config;
    config.iceServers.emplace_back("stun:stun.l.google.com:19302");

    peer_connection = std::make_unique<rtc::PeerConnection>(config);
    peer_connection->onLocalCandidate([this](rtc::Candidate candidate)
                                      { 
                                        if(ice_state != rtc::PeerConnection::IceState::Completed)
                                            ice_generate_cb(candidate); });
    peer_connection->onIceStateChange([this](rtc::PeerConnection::IceState state)
                                      {    
        switch (state) {
            case rtc::PeerConnection::IceState::New:
                ice_status_cb(IceState::New);
                break;
            case rtc::PeerConnection::IceState::Checking:
                ice_status_cb(IceState::Checking);
                break;
            case rtc::PeerConnection::IceState::Connected:
                ice_status_cb(IceState::Connected);
                break;
                case rtc::PeerConnection::IceState::Completed:
                ice_status_cb(IceState::Completed);
                break;
            case rtc::PeerConnection::IceState::Failed:
                ice_status_cb(IceState::Failed);
                break;
                case rtc::PeerConnection::IceState::Disconnected:
                ice_status_cb(IceState::Disconnected);
                break;
                case rtc::PeerConnection::IceState::Closed:
                LOG_ERROR("rtc::PeerConnection::IceState::Closed");
                ice_status_cb(IceState::Closed);
                break;
            default:
                break;
        } });
}

// 用户数据
void P2PDriver::connect(const std::string &addr, const std::string &p, std::function<void(bool)> callback)
{
    websocket_driver->connect(addr, p, callback);
}

void P2PDriver::sendMsg(const std::string &msg, std::string label)
{
    LOG_DEBUG("InP2P Send: " << msg << "  Label: " << label);
    if (!label.empty())
    {
        label_dc_map[label]->send(msg);
    }
}

void P2PDriver::recvMsg(std::function<void(std::string)> callback)
{
    LOG_DEBUG("start user recv");
    msg_callback = callback;
    label_dc_map["user"]->onMessage([=](rtc::message_variant data)
                                    { callback(std::get<std::string>(data)); });
}

void P2PDriver::closeSocket()
{
    for (const auto &it : label_dc_map)
    {
        if (it.second && it.second->isOpen())
        {
            it.second->close();
        }
    }
    peer_connection->close();
}

void P2PDriver::resetConnection()
{
    closeSocket();
}

void P2PDriver::createOffer(std::function<void(const std::string &offer)> callback)
{
    peer_connection->onLocalDescription([&](rtc::Description description)
                                        { callback(description); });
    label_dc_map["user"] = peer_connection->createDataChannel("user");
    for (int i = 0; i < 5; ++i)
    {
        std::string sender_dn = "offer-answer" + std::to_string(i);
        std::string receiver_dn = "answer-offer" + std::to_string(i);
        label_dc_map.insert({sender_dn, peer_connection->createDataChannel(sender_dn)});
        label_dc_map.insert({receiver_dn, peer_connection->createDataChannel(receiver_dn)});
    }
    if (current_role == Role::Offer)
        recvMsg(msg_callback);
    current_role = Role::Offer;
}

void P2PDriver::setRemoteDescription(const std::string &sdp,
                                     std::function<void(bool, const std::string &answer)> callback)
{
    try
    {
        if (!peer_connection)
        {
            if (callback)
                callback(false, "");
            return;
        }
        peer_connection->onLocalDescription([this, callback](rtc::Description description)
                                            {            
            if (callback) {//callback不为空时，说明是answer，因为需要发送answer，offer方则直接设置answer即可
                current_role = Role::Answer;
                peer_connection->onDataChannel(std::bind(&P2PDriver::receiveDataChannel,this,std::placeholders::_1));
                callback(true, description);
            } });
        auto description = rtc::Description(sdp, "offer");

        peer_connection->setRemoteDescription(description);
    }
    catch (const std::exception &e)
    {
        if (callback)
        {
            callback(false, "");
        }
    }
}

void P2PDriver::addIceCandidate(const std::string &candidate)
{
    peer_connection->addRemoteCandidate(candidate);
}

void P2PDriver::receiveDataChannel(std::shared_ptr<rtc::DataChannel> dc)
{
    std::string label = dc->label();
    if (label == "user")
    {
        label_dc_map["user"] = dc;
        recvMsg(msg_callback);
    }
}
