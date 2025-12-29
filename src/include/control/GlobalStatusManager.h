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
        AutoDownloadThreshold,
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

    inline static constexpr const char *zh_change_log = R"(
    当前版本
    新增:
    一. 增加设置窗口,
      1.支持开机自启选项
      2.支持深浅主题切换
      3.支持中英语言切换
      4.支持自动清理缓存
      5.支持选择缓存路径
      6.支持手动清理缓存
      7.支持自动展开
      8.支持选择自动下载
      9.支持设置并行任务数
      10.支持检查更新，在线更新
    二.优化传输体验
      1.支持取消传输任务
      2.支持停止正在传输的任务
      3.增加下载全部，停止全部按钮)";

    inline static constexpr const char *en_change_log = R"(
    Current Version
    New Features:
     一、Add Settings Window
      1. Support startup on boot option
      2. Support light/dark theme switching
      3. Support Chinese/English language switching
      4. Support automatic cache cleanup
      5. Support selecting cache path
      6. Support manual cache cleanup
      7. Support auto-expand
      8. Support selecting automatic download
      9. Support setting the number of parallel tasks
      10. Support update check and online updates
    二、Optimize Transfer Experience
      1. Support canceling transfer tasks
      2. Support stopping ongoing transfer tasks
      3. Add Download All and Stop All buttons)";
}

#endif