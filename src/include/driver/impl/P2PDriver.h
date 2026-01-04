#ifndef _P2PDRIVER_H
#define _P2PDRIVER_H

#include "driver/interface/NetworkInterface.h"
#include "driver/interface/FileSyncEngine/FileReceiverInterface.h"
#include "driver/interface/FileSyncEngine/FileSenderInterface.h"

class P2PDriver : public NetworkInterface, public FileReceiverInterface, public FileSenderInterface
{
public:
    P2PDriver();
    // 用户数据
    void setNetworkInfo(const std::string &address, const std::string &port) override;
    void connectTo(std::function<void(bool)> callback = nullptr) override;
    void sendMsg(const std::string &msg) override;
    void recvMsg(std::function<void(std::unique_ptr<UserMsg>)> callback) override;
    void closeSocket() override;
    void resetConnection() override;

    // 文件接收接口
    void start(std::function<void(UnifiedSocket)> accept_cb,
               std::function<void(UnifiedSocket socket, std::unique_ptr<NetworkInterface::UserMsg>)> msg_cb) override;

    // 文件发送接口
    void start(std::function<std::optional<std::pair<uint32_t, std::string>>()> get_task_cb) override;

private:
    std::string signal_address;
    std::string signal_port;
    std::unique_ptr<NetworkInterface> websocket_driver;
};

#endif