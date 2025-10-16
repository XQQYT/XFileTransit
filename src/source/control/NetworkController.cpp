#include "control/NetworkController.h"
#include "control/EventBusManager.h"

void NetworkController::initSubscribe()
{
    EventBusManager::instance().subscribe("/network/send_connect_request",
        std::bind(&NetworkController::onSendConnectRequest,
            this,
            std::placeholders::_1,
            std::placeholders::_2));
}

NetworkController::NetworkController() :
    control_msg_network_driver(std::make_unique<TcpDriver>())
{
    initSubscribe();
}
void NetworkController::onSendConnectRequest(std::string device_name, std::string device_ip)
{
    std::cout << device_name << device_ip << std::endl;
}