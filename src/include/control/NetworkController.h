#ifndef _NETWORKCONTROLLER_H
#define _NETWORKCONTROLLER_H

#include "driver/interface/Network/TcpInterface.h"
#include "driver/interface/Network/P2PInterface.h"
#include "driver/interface/JsonFactoryInterface.h"
#include "driver/interface/SecurityInterface.h"
#include "MsgParser/Parser.h"

class NetworkController
{
public:
    void initSubscribe();
    NetworkController();

private:
    void onSendConnectRequest(std::unordered_map<std::string, std::string> args);
    void onSendConnectRequestResult(bool res);
    void onResetConnection();
    void onHaveConnectRequestResult(bool res, std::string);
    void onDisconnect();
    void onConnectError(const TcpInterface::ConnectError error);
    void onRecvError(const TcpInterface::RecvError error);
    void onConnClosed();
    void onSendExpiredFile(uint32_t id);
    void onSendSyncAddFiles(std::vector<std::string> files, uint8_t stride);
    void onSendSyncDeleteFile(uint32_t id);
    void onSendGetFile(uint32_t id);
    void onConcurrentChanged(uint8_t num);
    void onSendCancelTransit(uint32_t id);
    void onSendInitFileReceiverDone();
    void onSetEncrptyed(bool enable);

private:
    std::unique_ptr<TcpInterface> tcp_driver;
    std::shared_ptr<NetworkInterface> websocket_driver;
    std::shared_ptr<P2PInterface> p2p_driver;
    std::shared_ptr<NetworkInterface> network_driver;
    std::unique_ptr<Json::JsonFactoryInterface> user_json_builder;
    std::unique_ptr<Json::JsonBuilder> signal_json_builder;
    std::shared_ptr<SecurityInterface> security_driver;
    Parser json_parser;
};

#endif