#include "control/NetworkController.h"
#include "control/EventBusManager.h"
#include "driver/impl/TcpDriver.h"
#include "driver/impl/Nlohmann.h"
#include "driver/impl/OpensslDriver.h"

void NetworkController::initSubscribe()
{
    EventBusManager::instance().subscribe("/network/send_connect_request",
        std::bind(&NetworkController::onSendConnectRequest,
            this,
            std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3));
}

NetworkController::NetworkController() :
    control_msg_network_driver(std::make_unique<TcpDriver>()),
    json_builder(std::make_unique<NlohmannJson>()),
    security_driver(std::make_shared<OpensslDriver>())
{
    initSubscribe();
    // control_msg_network_driver->setSecurityInstance(security_driver);
    control_msg_network_driver->startListen("0.0.0.0", "7777", [this](bool connect_status) -> bool
        {
            std::cout << "accepted" << std::endl;
            control_msg_network_driver->recvMsg([](NetworkInterface::ParsedMsg&& msg)
                {
                    std::cout << "recv msg -> " << std::string(msg.data.data(), msg.data.data() + msg.data.size()) << std::endl;
                });
            return true;
        });
}
void NetworkController::onSendConnectRequest(std::string sender_device_name, std::string sender_device_ip, std::string target_device_ip)
{
    control_msg_network_driver->initSocket(target_device_ip, "7777");
    control_msg_network_driver->connectTo([=](bool ret)
        {
            if (ret)
            {
                std::cout << "build" << std::endl;
                std::string msg = json_builder->getBuilder(Json::BuilderType::User)->build(
                    static_cast<uint64_t>(Json::MessageType::User::ConnectRequest),
                    {
                         {"sender_device_name",std::move(sender_device_name)},
                         {"sender_device_ip",std::move(sender_device_ip)}
                    });
                std::cout << "send" << std::endl;
                control_msg_network_driver->sendMsg(msg);
                std::cout << "sended" << std::endl;
            }
            else
            {
                std::cout << "failed to connect" << std::endl;
            }
        });
}