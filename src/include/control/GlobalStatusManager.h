#ifndef _GLOBALSTATUSMANAGER_H
#define _GLOBALSTATUSMANAGER_H

#include <string>

class GlobalStatusManager
{
public:
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

private:
    std::string current_device_name;
    std::string current_device_ip;
};

#endif