#ifndef _GLOBALSTATUSMANAGER_H
#define _GLOBALSTATUSMANAGER_H

#include <string>
#include "driver/impl/FileUtility.h"

class GlobalStatusManager
{
public:
    inline static std::string absolute_tmp_dir = FileSystemUtils::getExecutableDirectory() + "XFiletransitTmp/";

    enum class idType
    {
        Low,
        High,
        Undefined
    };
    static GlobalStatusManager &getInstance()
    {
        static GlobalStatusManager instance;
        return instance;
    }
    inline std::string getCurrentTargetDeviceName()
    {
        return current_target_device_name;
    }
    inline std::string getCurrentTargetDeviceIP()
    {
        return current_target_device_ip;
    }
    inline void setCurrentTargetDeviceName(std::string &&dn)
    {
        current_target_device_name = dn;
    }
    inline void setCurrentTargetDeviceIP(const std::string &di)
    {
        current_target_device_ip = di;
    }
    inline std::string getCurrentLocalDeviceName()
    {
        return current_local_device_name;
    }
    inline std::string getCurrentLocalDeviceIP()
    {
        return current_local_device_ip;
    }
    inline void setCurrentLocalDeviceName(std::string &&dn)
    {
        current_local_device_name = dn;
    }
    inline void setCurrentLocalDeviceIP(std::string &&di)
    {
        current_local_device_ip = di;
    }
    inline void setConnectStatus(bool status)
    {
        is_connected = status;
    }
    inline bool getConnectStatus()
    {
        return is_connected;
    }
    inline uint32_t getFileId()
    {
        if (current_type == idType::Undefined)
        {
            throw std::runtime_error("id type is undefined");
        }
        if (current_type == idType::Low)
        {
            return file_id_counter++;
        }
        else
        {
            return file_id_counter--;
        }
    }
    inline void setIdBegin(idType type)
    {
        current_type = type;
        switch (type)
        {
        case idType::Low:
            file_id_counter = 0;
            break;
        case idType::High:
            file_id_counter = UINT32_MAX;
            break;
        default:
            break;
        }
    }
    inline void insertFile(uint32_t id, const std::string &filename)
    {
        id_filename.insert({id, filename});
    }
    inline void removeFile(uint32_t id)
    {
        id_filename.erase(id);
    }
    inline std::string getFileName(uint32_t id)
    {
        return id_filename.at(id);
    }
    inline void reset()
    {
        current_type = idType::Undefined;
    }

private:
    GlobalStatusManager()
    {
        FileSystemUtils::createDirectoryRecursive(absolute_tmp_dir);
    }
    std::string current_target_device_name;
    std::string current_target_device_ip;

    std::string current_local_device_name;
    std::string current_local_device_ip;
    bool is_connected{false};
    uint32_t file_id_counter;
    idType current_type;
    std::unordered_map<uint32_t, std::string> id_filename;
};

namespace Settings
{
    enum class Item
    {
        Theme,
        Language,
        CachePath,
        AutoClearCache,
        AutoDownload,
        ConcurrentTransfers,
        ExpandOnAction,
        AppVersion,
        IsUpdateAvailable,
        NewVersion
    };

    enum class SettingsGroup
    {
        General,
        File,
        Transfer,
        Notification,
        About
    };
    inline uint8_t to_uint8(SettingsGroup group)
    {
        return static_cast<uint8_t>(group);
    }
    constexpr const char *toString(SettingsGroup item)
    {
        switch (item)
        {
        case SettingsGroup::General:
            return "General";
        case SettingsGroup::File:
            return "File";
        case SettingsGroup::Transfer:
            return "Transfer";
        case SettingsGroup::Notification:
            return "Notification";
        case SettingsGroup::About:
            return "About";
        default:
            return "unknown";
        }
    }

    inline constexpr const char *config_file = "./settings.json";
    constexpr const uint8_t group_count = 5;
};

inline namespace AppVersion
{
    inline constexpr int Major = 1;
    inline constexpr int Minor = 0;
    inline constexpr int Patch = 0;
    inline constexpr int Build = 20241128;

    inline static constexpr const char *string = "v1.1.0";
    inline static constexpr const char *string_full = "v1.0.0.20251228";

    inline static constexpr const char *change_log = R"(
    当前版本
    新增:
    1. 局域网自动发现，无需服务器或繁琐配置，设备可自动互联。
    2. 文件拖拽到主窗口即可一键发送，支持多文件同时添加和传输。
    3. 实时显示传输进度，任务多线程处理，提升性能与体验。
    4. 支持文件列表去重与批量发送、批量接收。
    5. 支持自动同步当前文件列表，便于多用户协同。
    6. 实现文件传输的 TLS +ES 加密，保障数据安全。
    7. 提供连接状态检测及断开自动清理，确保稳定性。
    8. 支持传入和下载文件的请求。
    9. 便捷设备管理与连接管理，适应局域网高效使用场景。
    10. 支持文件夹与文件的识别与处理，完整本地路径管理。 
    11. 查看本地网络信息)";
}

#endif