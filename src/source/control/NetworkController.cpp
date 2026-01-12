#include "control/NetworkController.h"
#include "control/EventBusManager.h"
#include "driver/impl/Network/TcpDriver.h"
#include "driver/impl/Network/WebSocket.h"
#include "driver/impl/Network/P2PDriver.h"
#include "driver/impl/UserJsonDriver.h"
#include "driver/impl/SignalJsonDriver.h"
#include "driver/impl/OpensslDriver.h"
#include "control/MsgParser/UserJsonParser.h"
#include "control/MsgParser/SignalJsonParser.h"
#include "control/GlobalStatusManager.h"
#include "common/DebugOutputer.h"

void NetworkController::initSubscribe()
{
    EventBusManager::instance().subscribe("/network/set_encrptyed",
                                          std::bind(&NetworkController::onSetEncrptyed,
                                                    this,
                                                    std::placeholders::_1));
    EventBusManager::instance().subscribe("/network/send_connect_request",
                                          std::bind(&NetworkController::onSendConnectRequest,
                                                    this,
                                                    std::placeholders::_1));
    EventBusManager::instance().subscribe("/network/send_connect_request_result",
                                          std::bind(&NetworkController::onSendConnectRequestResult,
                                                    this,
                                                    std::placeholders::_1));
    EventBusManager::instance().subscribe("/network/reset_connection",
                                          std::bind(&NetworkController::onResetConnection,
                                                    this));
    EventBusManager::instance().subscribe("/network/cancel_conn_request", [this]()
                                          { tcp_driver->resetConnection(); });
    EventBusManager::instance().subscribe("/network/have_connect_request_result",
                                          std::bind(&NetworkController::onHaveConnectRequestResult,
                                                    this,
                                                    std::placeholders::_1,
                                                    std::placeholders::_2));
    EventBusManager::instance().subscribe("/network/disconnect",
                                          std::bind(&NetworkController::onDisconnect,
                                                    this));
    EventBusManager::instance().subscribe("/sync/send_expired_file",
                                          std::bind(&NetworkController::onSendExpiredFile,
                                                    this,
                                                    std::placeholders::_1));
    EventBusManager::instance().subscribe("/sync/send_addfiles",
                                          std::bind(&NetworkController::onSendSyncAddFiles,
                                                    this,
                                                    std::placeholders::_1,
                                                    std::placeholders::_2));
    EventBusManager::instance().subscribe("/sync/send_deletefiles",
                                          std::bind(&NetworkController::onSendSyncDeleteFile,
                                                    this,
                                                    std::placeholders::_1));
    EventBusManager::instance().subscribe("/file/send_get_file",
                                          std::bind(&NetworkController::onSendGetFile,
                                                    this,
                                                    std::placeholders::_1));
    EventBusManager::instance().subscribe("/settings/send_concurrent_changed",
                                          std::bind(&NetworkController::onConcurrentChanged,
                                                    this,
                                                    std::placeholders::_1));
    EventBusManager::instance().subscribe("/file/send_cancel_file_send",
                                          std::bind(&NetworkController::onSendCancelTransit,
                                                    this,
                                                    std::placeholders::_1));
    EventBusManager::instance().subscribe("/file/send_init_file_receiver_done",
                                          std::bind(&NetworkController::onSendInitFileReceiverDone,
                                                    this));

    // 设置错误处理回调函数
    tcp_driver->setDealConnectErrorCb(std::bind(&NetworkController::onConnectError, this, std::placeholders::_1));
    tcp_driver->setDealRecvErrorCb(std::bind(
        &NetworkController::onRecvError,
        this,
        std::placeholders::_1));
    tcp_driver->setDealConnClosedCb(std::bind(
        &NetworkController::onConnClosed,
        this));
}

NetworkController::NetworkController() : tcp_driver(std::make_shared<TcpDriver>()),
                                         p2p_driver(std::make_shared<P2PDriver>()),
                                         websocket_driver(WebSocket::create()),
                                         user_json_builder(std::make_unique<NlohmannJson>()),
                                         signal_json_builder(std::make_unique<SignalJsonMsgBuilder>()),
                                         security_driver(std::make_shared<OpensslDriver>())
{
    initSubscribe();
    // 设置安全实例驱动才会按照加密协议进行通信
    tcp_driver->setSecurityInstance(security_driver);
    tcp_driver->startListen("0.0.0.0", "7777", "7778", nullptr, [this](bool connect_status) -> bool
                            {
            tcp_driver->recvMsg([this](std::unique_ptr<NetworkInterface::UserMsg> msg)
                {
                    json_parser.parse(std::move(msg));
                });
            return true; });
    websocket_driver->connect("192.168.2.117", "8888", [=](bool ret)
                              { 
        if(ret)
        {
            json_parser.setP2PInstance(p2p_driver);
            json_parser.setWSInstance(websocket_driver);
            p2p_driver->setMsgParser([=](std::string msg){
                json_parser.parse(msg, Parser::MsgType::User);
            });
            websocket_driver->recvMsg([=](std::string msg)
                { json_parser.parse(msg, Parser::MsgType::Signal); });
            json_parser.setOnP2PStatusChanged([=](P2PInterface::IceState state){
                if(state == P2PInterface::IceState::Completed)
                {
                    GlobalStatusManager::getInstance().setConnectStatus(true);
                    network_driver = p2p_driver;
                }
            });
            websocket_driver->sendMsg(signal_json_builder->buildSignalMsg(Json::MessageType::Signal::Register, {{"id", ConnectionInfo::my_code}}));
        } });
}

void NetworkController::onSetEncrptyed(bool enable)
{
    OuterMsgBuilderInterface::encrptyed = enable;
}

void NetworkController::onSendConnectRequest(std::unordered_map<std::string, std::string> args)
{
    switch (ConnectionInfo::connection_type)
    {
    case ConnectionInfo::Tcp:
    {
        std::string sender_ip = args["sender_ip"];
        std::string sender_name = args["sender_name"];
        std::string target_ip = args["target_ip"];

        std::cout << tcp_driver.get() << std::endl;
        tcp_driver->setTlsNetworkInfo(target_ip, "7777");
        tcp_driver->connect(args["target_ip"], "7778", [=](bool ret)
                            {
            if (ret)
            {
                tcp_driver->recvMsg([this](std::unique_ptr<NetworkInterface::UserMsg> msg)
                    {
                        json_parser.parse(std::move(msg));
                    });
                GlobalStatusManager::getInstance().setCurrentTargetDeviceIP(target_ip);
                std::string msg = user_json_builder->getBuilder(Json::BuilderType::User)->buildUserMsg(
                    Json::MessageType::User::ConnectRequest,
                    {
                         {"sender_device_name",sender_name},
                         {"sender_device_ip",sender_ip}
                    });
                tcp_driver->sendMsg(msg);
            }
            else
            {
                LOG_ERROR("failed to connect");
            } });
        GlobalStatusManager::getInstance().setCurrentLocalDeviceIP(std::move(sender_ip));
        GlobalStatusManager::getInstance().setCurrentLocalDeviceName(std::move(sender_name));
        break;
    }
    case ConnectionInfo::P2P:
    {
        std::string target_code = args["target_code"];
        TargetInfo::target_code = target_code;
        TargetInfo::target_password = args["password"];

        std::string signal_msg;
        switch (TargetInfo::target_status)
        {
        case TargetStatus::WaitingStatus:
            signal_msg = signal_json_builder->buildSignalMsg(
                Json::MessageType::Signal::GetTargetStatus,
                {{"target_code", target_code},
                 {"sender_code", ConnectionInfo::my_code}});
            break;
        case TargetStatus::Online:
            signal_msg = signal_json_builder->buildSignalMsg(
                Json::MessageType::Signal::ConnectRequest,
                {{"target_code", target_code},
                 {"password", target_password},
                 {"sender_code", ConnectionInfo::my_code}});
            break;
        default:
            return;
        }
        websocket_driver->sendMsg(std::move(signal_msg));
        break;
    }
    break;
    default:
        break;
    }
}

// 发送ip和name，预留扩展
void NetworkController::onResetConnection()
{
    std::string msg = user_json_builder->getBuilder(Json::BuilderType::User)->buildUserMsg(Json::MessageType::User::CancelConnRequest, {{"sender_device_name", GlobalStatusManager::getInstance().getCurrentLocalDeviceName()}, {"sender_device_ip", GlobalStatusManager::getInstance().getCurrentLocalDeviceIP()}});
    network_driver->sendMsg(msg, "user");
    network_driver->resetConnection();
}

void NetworkController::onSendConnectRequestResult(bool res)
{

    switch (ConnectionInfo::connection_type)
    {
    case ConnectionType::Tcp:
    {
        std::string msg = user_json_builder->getBuilder(Json::BuilderType::User)->buildUserMsg(Json::MessageType::User::ConnectRequestResponse, {{"subtype", "connect_request_response"}, {"arg0", res ? "success" : "failed"}});
        tcp_driver->sendMsg(msg);
        if (res) // 接受连接
        {
            GlobalStatusManager::getInstance().setIdBegin(GlobalStatusManager::idType::High);
            EventBusManager::instance().publish("/file/initialize_FileSyncCore",
                                                GlobalStatusManager::getInstance().getCurrentTargetDeviceIP(), std::string("7779"), security_driver);
            network_driver = tcp_driver;
        }
        else // 拒绝连接
        {
            tcp_driver->resetConnection();
        }
        GlobalStatusManager::getInstance().setConnectStatus(res);
        break;
    }
    case ConnectionType::P2P:
    {
        std::string msg = signal_json_builder->buildSignalMsg(Json::MessageType::Signal::ConnectRequestResult,
                                                              {{"sender_code", ConnectionInfo::my_code},
                                                               {"result", res ? "true" : "false"},
                                                               {"target_code", TargetInfo::target_code}});
        websocket_driver->sendMsg(msg);
        if (res) // 接受连接
        {
            GlobalStatusManager::getInstance().setIdBegin(GlobalStatusManager::idType::High);
        }
        GlobalStatusManager::getInstance().setConnectStatus(res);
        break;
    }
    }
}

void NetworkController::onHaveConnectRequestResult(bool res, std::string)
{
    switch (ConnectionInfo::connection_type)
    {
    case ConnectionType::Tcp:
        if (res)
        {
            GlobalStatusManager::getInstance().setIdBegin(GlobalStatusManager::idType::Low);
            EventBusManager::instance().publish("/file/initialize_FileSyncCore",
                                                GlobalStatusManager::getInstance().getCurrentTargetDeviceIP(), std::string("7779"), security_driver);
            network_driver = tcp_driver;
        }
        else
        {
            tcp_driver->resetConnection();
        }
        GlobalStatusManager::getInstance().setConnectStatus(res);
        break;
    case ConnectionType::P2P:
        p2p_driver->createOffer([=](const std::string &str)
                                { websocket_driver->sendMsg(signal_json_builder->buildSignalMsg(
                                      Json::MessageType::Signal::Offer,
                                      {{"offer", str},
                                       {"target_code", TargetInfo::target_code},
                                       {"sender_code", ConnectionInfo::my_code}})); });
        break;
    }
}

void NetworkController::onDisconnect()
{
    network_driver->resetConnection();
    GlobalStatusManager::getInstance().setConnectStatus(false);
    EventBusManager::instance().publish("/file/close_FileSyncCore");
}

void NetworkController::onConnectError(const TcpInterface::ConnectError error)
{
    static const std::map<TcpInterface::ConnectError, std::string> connect_error_messages = {
        {TcpInterface::ConnectError::CONNECT_ACCESS_DENIED, "连接被拒绝：权限不足"},
        {TcpInterface::ConnectError::CONNECT_ADDR_IN_USE, "地址已被占用"},
        {TcpInterface::ConnectError::CONNECT_ALREADY_CONNECTED, "套接字已连接"},
        {TcpInterface::ConnectError::CONNECT_BAD_ADDRESS, "地址参数错误"},
        {TcpInterface::ConnectError::CONNECT_HOST_UNREACHABLE, "目标主机不可达"},
        {TcpInterface::ConnectError::CONNECT_IN_PROGRESS, "非阻塞连接正在进行中"},
        {TcpInterface::ConnectError::CONNECT_INTERRUPTED, "连接操作被中断"},
        {TcpInterface::ConnectError::CONNECT_NETWORK_UNREACHABLE, "网络不可达"},
        {TcpInterface::ConnectError::CONNECT_REFUSED, "连接被目标拒绝"},
        {TcpInterface::ConnectError::CONNECT_TIMEOUT, "连接超时"}};

    // 使用时
    auto it = connect_error_messages.find(error);
    if (it != connect_error_messages.end())
    {
        EventBusManager::instance().publish("/network/have_connect_error", it->second);
    }
    else
    {
        EventBusManager::instance().publish("/network/have_connect_error", std::string("未知连接错误"));
    }
    network_driver->resetConnection();
    GlobalStatusManager::getInstance().setConnectStatus(false);
    GlobalStatusManager::getInstance().setIdBegin(GlobalStatusManager::idType::Low);
    EventBusManager::instance().publish("/file/close_FileSyncCore");
}

void NetworkController::onRecvError(const TcpInterface::RecvError error)
{
    static const std::map<TcpInterface::RecvError, std::string> error_map = {
        {TcpInterface::RecvError::RECV_CONN_ABORTED, "连接被中止"},
        {TcpInterface::RecvError::RECV_NOT_CONNECTED, "网络未连接"},
        {TcpInterface::RecvError::RECV_NETWORK_DOWN, "网络连接故障"},
        {TcpInterface::RecvError::RECV_TIMED_OUT, "接收数据超时"},
        {TcpInterface::RecvError::RECV_INTERRUPTED, "接收操作被中断"},
        {TcpInterface::RecvError::RECV_SHUTDOWN, "连接已关闭"},
        {TcpInterface::RecvError::RECV_NETWORK_RESET, "网络连接重置"}};

    auto it = error_map.find(error);
    if (it != error_map.end())
    {
        EventBusManager::instance().publish("/network/have_recv_error", it->second);
    }
    else
    {
        EventBusManager::instance().publish("/network/have_recv_error", std::string("未知接收错误"));
    }
    network_driver->resetConnection();
    GlobalStatusManager::getInstance().setConnectStatus(false);
    GlobalStatusManager::getInstance().setIdBegin(GlobalStatusManager::idType::Low);
    EventBusManager::instance().publish("/file/close_FileSyncCore");
}

void NetworkController::onConnClosed()
{
    EventBusManager::instance().publish("/network/connection_closed");
    // 重置驱动上下文
    network_driver->resetConnection();
    GlobalStatusManager::getInstance().setConnectStatus(false);
    GlobalStatusManager::getInstance().setIdBegin(GlobalStatusManager::idType::Low);
    EventBusManager::instance().publish("/file/close_FileSyncCore");
}

void NetworkController::onSendExpiredFile(uint32_t id)
{
    auto sync_builder = user_json_builder->getBuilder(Json::BuilderType::Sync);
    network_driver->sendMsg(
        sync_builder->buildSyncMsg(Json::MessageType::Sync::FileExpired, {std::to_string(id)}, 1), "user");
}

void NetworkController::onSendSyncAddFiles(std::vector<std::string> files, uint8_t stride)
{
    auto sync_builder = user_json_builder->getBuilder(Json::BuilderType::Sync);
    network_driver->sendMsg(
        sync_builder->buildSyncMsg(Json::MessageType::Sync::AddFiles, std::move(files), stride), "user");
}

void NetworkController::onSendSyncDeleteFile(uint32_t id)
{
    auto sync_builder = user_json_builder->getBuilder(Json::BuilderType::Sync);
    network_driver->sendMsg(
        sync_builder->buildSyncMsg(Json::MessageType::Sync::RemoveFile, {std::to_string(id)}, 1), "user");
}

void NetworkController::onSendGetFile(uint32_t id)
{
    auto sync_builder = user_json_builder->getBuilder(Json::BuilderType::Sync);
    network_driver->sendMsg(
        sync_builder->buildSyncMsg(Json::MessageType::Sync::DownloadFile, {std::to_string(id)}, 1), "user");
}

void NetworkController::onConcurrentChanged(uint8_t num)
{
    auto settings_builder = user_json_builder->getBuilder(Json::BuilderType::Settings);
    network_driver->sendMsg(
        settings_builder->buildSettingsMsg(Json::MessageType::Settings::ConcurrentTask, {{"concurrent", std::to_string(num)}}), "user");
}

void NetworkController::onSendCancelTransit(uint32_t id)
{
    auto file_builder = user_json_builder->getBuilder(Json::BuilderType::File);
    network_driver->sendMsg(
        file_builder->buildFileMsg(Json::MessageType::File::FileCancel, {{"id", std::to_string(id)}}), "user");
}

void NetworkController::onSendInitFileReceiverDone()
{
    auto file_builder = user_json_builder->getBuilder(Json::BuilderType::File);
    network_driver->sendMsg(
        file_builder->buildFileMsg(Json::MessageType::File::ReceiverInitDone, {}), "user");
}