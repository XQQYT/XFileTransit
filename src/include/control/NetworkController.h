#ifndef _NETWORKCONTROLLER_H
#define _NETWORKCONTROLLER_H

#include "driver/impl/TcpDriver.h"

class NetworkController
{
public:
    void initSubscribe();
    NetworkController();
private:
    void onSendConnectRequest(std::string device_name, std::string device_ip);
private:
    std::unique_ptr<NetworkInterface> control_msg_network_driver;
};

#endif