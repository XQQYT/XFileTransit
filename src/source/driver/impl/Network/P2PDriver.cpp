#include "driver/impl/Network/P2PDriver.h"
#include "driver/impl/Network/WebSocket.h"
#include "driver/impl/FileSyncEngine/FileParser.h"
#include "driver/impl/OuterMsgParser.h"
#include "common/DebugOutputer.h"
#include <rtc/configuration.hpp>

P2PDriver::P2PDriver()
    : websocket_driver(WebSocket::create()),
      outer_parser(std::make_unique<OuterMsgParser>())
{
    current_role = Role::Default;
    for (int i = 0; i < 5; ++i)
    {
        file_parsers.push_back(std::make_unique<FileParser>());
    }
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

void P2PDriver::sendMsg(const std::vector<uint8_t> &msg, std::string label)
{
    if (!label.empty())
    {
        label_dc_map[label]->sendBuffer(msg);
    }
}

void P2PDriver::recvMsg(std::function<void(std::string)> callback)
{
    LOG_DEBUG("start user recv");
    msg_callback = callback;
    label_dc_map["user"]->onMessage([=](rtc::message_variant data)
                                    { 
                                        LOG_DEBUG("user收到: "<<std::get<std::string>(data));
                                        callback(std::get<std::string>(data)); });
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
    static int parser_index = 0;
    peer_connection->onLocalDescription([&](rtc::Description description)
                                        { callback(description); });
    label_dc_map["user"] = peer_connection->createDataChannel("user");
    for (int i = 0; i < 5; ++i)
    {
        std::string sender_dn = "offer-answer" + std::to_string(i);
        std::string receiver_dn = "answer-offer" + std::to_string(i);
        label_dc_map.insert({sender_dn, peer_connection->createDataChannel(sender_dn)});
        label_dc_map.insert({receiver_dn, peer_connection->createDataChannel(receiver_dn)});
        label_dc_map[receiver_dn]->onMessage(std::bind(&P2PDriver::parseRecvMsg, this, std::placeholders::_1));
        sender_label_list.push_back(sender_dn);
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

// answer方
void P2PDriver::receiveDataChannel(std::shared_ptr<rtc::DataChannel> dc)
{
    static int parser_index = 0;
    std::string label = dc->label();

    label_dc_map[label] = dc;

    if (label == "user")
    {
        recvMsg(msg_callback);
    }
    else if (label.substr(0, 5) == "offer")
    {
        dc->onMessage(std::bind(&P2PDriver::parseRecvMsg, this, std::placeholders::_1));
    }
    else if (label.substr(0, 6) == "answer")
    {
        sender_label_list.push_back(label);
    }
}

void P2PDriver::parseRecvMsg(rtc::message_variant msg)
{
    auto byte_msg = std::get<std::vector<std::byte>>(msg);
    if (byte_msg.size() < sizeof(Header))
    {
        LOG_ERROR("Bad Data");
        return;
    }
    std::vector<uint8_t> header;
    header.reserve(sizeof(Header));
    std::transform(byte_msg.data(), byte_msg.data() + sizeof(Header),
                   std::back_inserter(header),
                   [](std::byte b)
                   { return std::to_integer<uint8_t>(b); });

    std::vector<uint8_t> data;
    data.reserve(byte_msg.size() - sizeof(Header));
    std::transform(byte_msg.data() + sizeof(Header), byte_msg.data() + byte_msg.size(),
                   std::back_inserter(data),
                   [](std::byte b)
                   { return std::to_integer<uint8_t>(b); });
    if (header[0] == 0xAB && header[1] == 0xCD)
    {
        uint8_t version = header[2];
        uint32_t length = 0;
        memcpy(&length, header.data() + 3, 4);
        uint8_t flag = header[7];

        if (data.size() < length)
        {
            LOG_ERROR("Data length invalid");
            return;
        }
        else if (data.size() > length)
        {
            LOG_WARN("Data length bigger than length in header");
            outer_parser->parse(std::move(data), length, flag);
        }
        else
        {
            outer_parser->parse(std::move(data), length, flag);
        }
    }
    else
    {
        LOG_ERROR("Bad magic");
        return;
    }
}
