#include "control/NetworkController.h"
#include "control/EventBusManager.h"
#include "driver/impl/TcpDriver.h"
#include "driver/impl/Nlohmann.h"
#include "driver/impl/OpensslDriver.h"
#include "control/MsgParser/JsonParser.h"
#include "control/MsgParser/BinaryParser.h"

void NetworkController::initSubscribe()
{
    EventBusManager::instance().subscribe("/network/send_connect_request",
        std::bind(&NetworkController::onSendConnectRequest,
            this,
            std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3));
    EventBusManager::instance().subscribe("/network/send_connect_request_result",
        std::bind(&NetworkController::onSendConnectRequestResult,
            this,
            std::placeholders::_1));
    EventBusManager::instance().subscribe("/network/have_connect_request_result",
        std::bind(&NetworkController::onHaveConnectRequestResult,
            this,
            std::placeholders::_1));
}

NetworkController::NetworkController() :
    control_msg_network_driver(std::make_unique<TcpDriver>()),
    json_builder(std::make_unique<NlohmannJson>()),
    security_driver(std::make_shared<OpensslDriver>()),
    json_parser(std::make_unique<JsonParser>())
{
    initSubscribe();
    control_msg_network_driver->setSecurityInstance(security_driver);
    control_msg_network_driver->startListen("0.0.0.0", "7777","7778", nullptr ,[this](bool connect_status) -> bool
        {
            control_msg_network_driver->recvMsg([this](NetworkInterface::ParsedMsg&& msg)
                {
                    std::cout << "recv msg -> " << std::string(msg.data.data(), msg.data.data() + msg.data.size()) << std::endl;
                    json_parser->parse(std::move(msg.data));
                });
            return true;
        });
}

void NetworkController::onSendConnectRequest(std::string sender_device_name, std::string sender_device_ip, std::string target_device_ip)
{
    control_msg_network_driver->initSocket(target_device_ip, "7777", "7778");
    control_msg_network_driver->connectTo([=](bool ret)
        {
            if (ret)
            {
                control_msg_network_driver->recvMsg([this](NetworkInterface::ParsedMsg&& msg)
                {
                    std::cout << "recv msg -> " << std::string(msg.data.data(), msg.data.data() + msg.data.size()) << std::endl;
                    json_parser->parse(std::move(msg.data));
                });
                std::string msg = json_builder->getBuilder(Json::BuilderType::User)->build(
                    static_cast<uint64_t>(Json::MessageType::User::ConnectRequest),
                    {
                         {"sender_device_name",std::move(sender_device_name)},
                         {"sender_device_ip",std::move(sender_device_ip)}
                    });
                control_msg_network_driver->sendMsg(msg);
            }
            else
            {
                std::cout << "failed to connect" << std::endl;
            }
        });
}

void NetworkController::onSendConnectRequestResult(bool res)
{
    std::string msg = json_builder->getBuilder(Json::BuilderType::User)->build(
        static_cast<uint64_t>(Json::MessageType::User::ConnectRequestResponse),
        {
            {"subtype","connect_request_response"},
            {"arg0",res ? "success" : "failed"}
        }
    );
    control_msg_network_driver->sendMsg(msg);
    if(!res)
    {
        control_msg_network_driver->resetConnection();
    }
}

void NetworkController::onHaveConnectRequestResult(bool res)
{
    if(!res)
    {
        control_msg_network_driver->resetConnection();
    }
}
