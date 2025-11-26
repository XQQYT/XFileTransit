#include "control/NetworkController.h"
#include "control/EventBusManager.h"
#include "driver/impl/TcpDriver.h"
#include "driver/impl/Nlohmann.h"
#include "driver/impl/OpensslDriver.h"
#include "control/MsgParser/JsonParser.h"
#include "control/MsgParser/BinaryParser.h"
#include "control/GlobalStatusManager.h"

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
            std::placeholders::_1,
            std::placeholders::_2));
    EventBusManager::instance().subscribe("/network/disconnect",
        std::bind(&NetworkController::onDisconnect,
            this));
    EventBusManager::instance().subscribe("/sync/send_addfiles",
        std::bind(&NetworkController::onSendSyncAddFiles,
            this,
            std::placeholders::_1,
            std::placeholders::_2));
    EventBusManager::instance().subscribe("/sync/send_deletefiles",
        std::bind(&NetworkController::onSendSyncDeleteFile,
            this,
            std::placeholders::_1));
    //设置错误处理回调函数
    control_msg_network_driver->setDealConnectErrorCb(std::bind(
        &NetworkController::onConnectError,
        this,
        std::placeholders::_1));
    control_msg_network_driver->setDealRecvErrorCb(std::bind(
        &NetworkController::onRecvError,
        this,
        std::placeholders::_1));
    control_msg_network_driver->setDealConnClosedCb(std::bind(
        &NetworkController::onConnClosed,
        this));
}

NetworkController::NetworkController() :
    control_msg_network_driver(std::make_unique<TcpDriver>()),
    json_builder(std::make_unique<NlohmannJson>()),
    security_driver(std::make_shared<OpensslDriver>()),
    json_parser(std::make_unique<JsonParser>())
{
    initSubscribe();
    //设置安全实例驱动才会按照加密协议进行通信
    control_msg_network_driver->setSecurityInstance(security_driver);
    control_msg_network_driver->startListen("0.0.0.0", "7777", "7778", nullptr, [this](bool connect_status) -> bool
        {
            control_msg_network_driver->recvMsg([this](std::unique_ptr<NetworkInterface::UserMsg> msg)
                {
                    std::cout << "recv msg -> " << std::string(msg->data.data(), msg->data.data() + msg->data.size()) << std::endl;
                    json_parser->parse(std::move(msg));
                });
            return true;
        });
}

void NetworkController::onSendConnectRequest(std::string sender_device_name, std::string sender_device_ip, std::string target_device_ip)
{
    control_msg_network_driver->initTlsSocket(target_device_ip, "7777");
    control_msg_network_driver->initTcpSocket(target_device_ip, "7778");
    control_msg_network_driver->connectTo([=](bool ret)
        {
            if (ret)
            {
                control_msg_network_driver->recvMsg([this](std::unique_ptr<NetworkInterface::UserMsg> msg)
                    {
                        std::cout << "recv msg -> " << std::string(msg->data.data(), msg->data.data() + msg->data.size()) << std::endl;
                        json_parser->parse(std::move(msg));
                    });
                GlobalStatusManager::getInstance().setCurrentDeviceIP(target_device_ip);
                std::string msg = json_builder->getBuilder(Json::BuilderType::User)->buildUserMsg(
                    Json::MessageType::User::ConnectRequest,
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
    std::string msg = json_builder->getBuilder(Json::BuilderType::User)->buildUserMsg(
        Json::MessageType::User::ConnectRequestResponse,
        {
            {"subtype","connect_request_response"},
            {"arg0",res ? "success" : "failed"}
        }
    );
    control_msg_network_driver->sendMsg(msg);
    if (res)//接受连接
    {
        GlobalStatusManager::getInstance().setIdBegin(GlobalStatusManager::idType::High);
    }
    else//拒绝连接
    {
        control_msg_network_driver->resetConnection();
    }
    GlobalStatusManager::getInstance().setConnectStatus(res);
}

void NetworkController::onHaveConnectRequestResult(bool res, std::string)
{
    if (res)
    {
        GlobalStatusManager::getInstance().setIdBegin(GlobalStatusManager::idType::Low);
    }
    else
    {
        control_msg_network_driver->resetConnection();
    }
    GlobalStatusManager::getInstance().setConnectStatus(res);
}

void NetworkController::onDisconnect()
{
    control_msg_network_driver->resetConnection();
    GlobalStatusManager::getInstance().setConnectStatus(false);
}

void NetworkController::onConnectError(const NetworkInterface::ConnectError error)
{
    static const std::map<NetworkInterface::ConnectError, std::string> connect_error_messages = {
        {NetworkInterface::ConnectError::CONNECT_ACCESS_DENIED, "连接被拒绝：权限不足"},
        {NetworkInterface::ConnectError::CONNECT_ADDR_IN_USE, "地址已被占用"},
        {NetworkInterface::ConnectError::CONNECT_ALREADY_CONNECTED, "套接字已连接"},
        {NetworkInterface::ConnectError::CONNECT_BAD_ADDRESS, "地址参数错误"},
        {NetworkInterface::ConnectError::CONNECT_HOST_UNREACHABLE, "目标主机不可达"},
        {NetworkInterface::ConnectError::CONNECT_IN_PROGRESS, "非阻塞连接正在进行中"},
        {NetworkInterface::ConnectError::CONNECT_INTERRUPTED, "连接操作被中断"},
        {NetworkInterface::ConnectError::CONNECT_NETWORK_UNREACHABLE, "网络不可达"},
        {NetworkInterface::ConnectError::CONNECT_REFUSED, "连接被目标拒绝"},
        {NetworkInterface::ConnectError::CONNECT_TIMEOUT, "连接超时"}
    };

    // 使用时
    auto it = connect_error_messages.find(error);
    if (it != connect_error_messages.end()) {
        EventBusManager::instance().publish("/network/have_connect_error", it->second);
    }
    else {
        EventBusManager::instance().publish("/network/have_connect_error", std::string("未知连接错误"));
    }
    control_msg_network_driver->resetConnection();
    GlobalStatusManager::getInstance().setConnectStatus(false);
    GlobalStatusManager::getInstance().setIdBegin(GlobalStatusManager::idType::Low);
}

void NetworkController::onRecvError(const NetworkInterface::RecvError error)
{
    static const std::map<NetworkInterface::RecvError, std::string> error_map = {
        {NetworkInterface::RecvError::RECV_CONN_ABORTED,    "连接被中止"},
        {NetworkInterface::RecvError::RECV_NOT_CONNECTED,   "网络未连接"},
        {NetworkInterface::RecvError::RECV_NETWORK_DOWN,    "网络连接故障"},
        {NetworkInterface::RecvError::RECV_TIMED_OUT,       "接收数据超时"},
        {NetworkInterface::RecvError::RECV_INTERRUPTED,     "接收操作被中断"},
        {NetworkInterface::RecvError::RECV_SHUTDOWN,        "连接已关闭"},
        {NetworkInterface::RecvError::RECV_NETWORK_RESET,   "网络连接重置"}
    };

    auto it = error_map.find(error);
    if (it != error_map.end()) {
        EventBusManager::instance().publish("/network/have_recv_error", it->second);
    }
    else {
        EventBusManager::instance().publish("/network/have_recv_error", std::string("未知接收错误"));
    }
    control_msg_network_driver->resetConnection();
    GlobalStatusManager::getInstance().setConnectStatus(false);
    GlobalStatusManager::getInstance().setIdBegin(GlobalStatusManager::idType::Low);
}

void NetworkController::onConnClosed()
{
    EventBusManager::instance().publish("/network/connection_closed");
    //重置驱动上下文
    control_msg_network_driver->resetConnection();
    GlobalStatusManager::getInstance().setConnectStatus(false);
    GlobalStatusManager::getInstance().setIdBegin(GlobalStatusManager::idType::Low);
}

void NetworkController::onSendSyncAddFiles(std::vector<std::string> files, uint8_t stride)
{
    auto sync_builder = json_builder->getBuilder(Json::BuilderType::Sync);
    control_msg_network_driver->sendMsg(
        sync_builder->buildSyncMsg(Json::MessageType::Sync::AddFiles, std::move(files)
            , stride));
}

void NetworkController::onSendSyncDeleteFile(uint32_t id)
{
    auto sync_builder = json_builder->getBuilder(Json::BuilderType::Sync);
    control_msg_network_driver->sendMsg(
        sync_builder->buildSyncMsg(Json::MessageType::Sync::RemoveFile, { std::to_string(id) }, 1));
}