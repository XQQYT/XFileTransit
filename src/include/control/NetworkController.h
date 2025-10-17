#ifndef _NETWORKCONTROLLER_H
#define _NETWORKCONTROLLER_H

#include "driver/interface/NetworkInterface.h"
#include "driver/interface/JsonFactoryInterface.h"

class NetworkController
{
public:
    void initSubscribe();
    NetworkController();
private:
    void onSendConnectRequest(std::string sender_device_name, std::string sender_device_ip, std::string target_device_ip);
private:
    std::unique_ptr<NetworkInterface> control_msg_network_driver;
    std::unique_ptr<Json::JsonFactoryInterface> json_builder;
};

#endif