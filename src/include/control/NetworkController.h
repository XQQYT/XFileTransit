#ifndef _NETWORKCONTROLLER_H
#define _NETWORKCONTROLLER_H

#include "driver/interface/NetworkInterface.h"
#include "driver/interface/JsonFactoryInterface.h"
#include "driver/interface/SecurityInterface.h"
#include "MsgParser/Parser.h"

class NetworkController
{
public:
    void initSubscribe();
    NetworkController();
private:
    void onSendConnectRequest(std::string sender_device_name, std::string sender_device_ip, std::string target_device_ip);
    void onSendConnectRequestResult(bool res);
    void onResetConnection();
    void onHaveConnectRequestResult(bool res, std::string);
    void onDisconnect();
    void onConnectError(const NetworkInterface::ConnectError error);
    void onRecvError(const NetworkInterface::RecvError error);
    void onConnClosed();
    void onSendExpiredFile(uint32_t id);
    void onSendSyncAddFiles(std::vector<std::string> files, uint8_t stride);
    void onSendSyncDeleteFile(uint32_t id);
    void onSendGetFile(uint32_t id);
private:
    std::unique_ptr<NetworkInterface> control_msg_network_driver;
    std::unique_ptr<Json::JsonFactoryInterface> json_builder;
    std::shared_ptr<SecurityInterface> security_driver;
    std::unique_ptr<Parser> json_parser;
    std::unique_ptr<Parser> binary_parser;
};

#endif