#ifndef _GLOBALSTATUSMANAGER_H
#define _GLOBALSTATUSMANAGER_H

#include <string>

class GlobalStatusManager
{
public:
    enum class idType {
        Low,
        High,
        Undefined
    };
    static GlobalStatusManager& getInstance()
    {
        static GlobalStatusManager instance;
        return instance;
    }
    std::string getCurrentDeviceName()
    {
        return current_device_name;
    }
    std::string getCurrentDeviceIP()
    {
        return current_device_ip;
    }
    void setCurrentDeviceName(std::string&& dn)
    {
        current_device_name = dn;
    }
    void setCurrentDeviceIP(std::string di)
    {
        current_device_ip = di;
    }
    void setConnectStatus(bool status)
    {
        is_connected = status;
    }
    bool getConnectStatus()
    {
        return is_connected;
    }
    uint32_t getFileId()
    {
        if(current_type == idType::Undefined)
        {
            throw std::runtime_error("id type is undefined");
        }
        if(current_type == idType::Low)
        {
            return file_id_counter++;
        }
        else
        {
            return file_id_counter--;
        }
    }
    void setIdBegin(idType type)
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
    void reset()
    {
        current_type = idType::Undefined;
    }

private:
    std::string current_device_name;
    std::string current_device_ip;
    bool is_connected{ false };
    uint32_t file_id_counter;
    idType current_type;
};

#endif