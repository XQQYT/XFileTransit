#ifndef _EVENTBUSMANAGER_H
#define _EVENTBUSMANAGER_H

#include "eventbus/EventBus.hpp"

class EventBusManager
{
public:
    static EventBusManager &instance()
    {
        static EventBusManager instance;
        return instance;
    }
    void startEventBus(const EventBus::EventBusConfig &cig = {})
    {
        // 默认配置
        if (cig.thread_model == EventBus::ThreadModel::UNDEFINED)
        {
            EventBus::EventBusConfig config;
            config.task_max = 1024;
            config.task_model = EventBus::TaskModel::NORMAL;
            config.thread_max = 8;
            config.thread_min = 2;
            config.thread_model = EventBus::ThreadModel::DYNAMIC;
            event_bus.initEventBus(config);
        }
        else
        {
            event_bus.initEventBus(cig);
        }
    }
    void registerEvent(const std::string &eventName)
    {
        event_bus.registerEvent(eventName);
    }
    template <typename Callback>
    callback_id subscribe(const std::string &eventName, Callback &&callback)
    {
        return event_bus.subscribe(eventName, std::forward<Callback>(callback));
    }
    template <typename... Args>
    void publish(const std::string &eventName, Args &&...args)
    {
        event_bus.publish(eventName, std::forward<Args>(args)...);
    }

private:
    EventBusManager() = default;
    EventBusManager(const EventBusManager &) = delete;
    EventBusManager &operator=(const EventBusManager &) = delete;
    EventBusManager(EventBusManager &&) = delete;
    EventBusManager &operator=(EventBusManager &&) = delete;

    EventBus event_bus;
};
#endif
