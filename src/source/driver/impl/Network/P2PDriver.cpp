#include "driver/impl/Network/P2PDriver.h"
#include "driver/impl/Network/WebSocket.h"
#include "common/DebugOutputer.h"
#include <rtc/configuration.hpp>

P2PDriver::P2PDriver()
    : websocket_driver(WebSocket::create())
{
}

void P2PDriver::initialize()
{
    rtc::Configuration config;
    config.iceServers.emplace_back("stun:stun.l.google.com:19302");

    peer_connection = std::make_unique<rtc::PeerConnection>(config);
}

void P2PDriver::setIceGenerateCb(std::function<void(const std::string &)> cb)
{
    peer_connection->onLocalCandidate([cb](rtc::Candidate candidate)
                                      { cb(candidate); });
}

// 用户数据
void P2PDriver::connect(const std::string &addr, const std::string &p, std::function<void(bool)> callback)
{
    websocket_driver->connect(addr, p, callback);
}

void P2PDriver::sendMsg(const std::string &msg)
{
    websocket_driver->sendMsg(msg);
}

void P2PDriver::recvMsg(std::function<void(std::string)> callback)
{
    websocket_driver->recvMsg(callback);
}

void P2PDriver::closeSocket()
{
}

void P2PDriver::resetConnection()
{
}

void P2PDriver::createOffer(std::function<void(const std::string &offer)> callback)
{
    peer_connection->onLocalDescription([&](rtc::Description description)
                                        { callback(description); });
    user_datachannel = peer_connection->createDataChannel("user");
    for (int i = 0; i < 5; ++i)
    {
        std::string sender_dn = "sender" + std::to_string(i);
        std::string receiver_dn = "receiver" + std::to_string(i);
        sender_datachannel.push_back(peer_connection->createDataChannel(sender_dn));
        receiver_datachannel.push_back(peer_connection->createDataChannel(receiver_dn));
    }
}

void P2PDriver::setRemoteDescription(const std::string &sdp,
                                     std::function<void(bool, const std::string &answer)> callback)
{
    try
    {
        if (!peer_connection)
        {
            std::cerr << "PeerConnection is null!" << std::endl;
            if (callback)
                callback(false, "");
            return;
        }
        peer_connection->onLocalDescription([this, callback](rtc::Description description)
                                            {            
            if (callback) {
                callback(true, description);
            } });
        auto description = rtc::Description(sdp, "offer");

        peer_connection->setRemoteDescription(description);

        std::cout << "Remote description set successfully" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Failed to set remote description: " << e.what() << std::endl;
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

void P2PDriver::setIceStatusCb(std::function<void(const IceState)> cb)
{
    peer_connection->onIceStateChange([cb](rtc::PeerConnection::IceState state)
                                      {    
        switch (state) {
            case rtc::PeerConnection::IceState::New:
                cb(IceState::New);
                break;
            case rtc::PeerConnection::IceState::Checking:
                cb(IceState::Checking);
                break;
            case rtc::PeerConnection::IceState::Connected:
                cb(IceState::Connected);
                break;
                case rtc::PeerConnection::IceState::Completed:
                cb(IceState::Completed);
                break;
            case rtc::PeerConnection::IceState::Failed:
                cb(IceState::Failed);
                break;
                case rtc::PeerConnection::IceState::Disconnected:
                cb(IceState::Disconnected);
                break;
                case rtc::PeerConnection::IceState::Closed:
                cb(IceState::Closed);
                break;
            default:
                break;
        } });
}
