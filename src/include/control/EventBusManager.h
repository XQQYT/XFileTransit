#ifndef _EVENTBUSMANAGER_H
#define _EVENTBUSMANAGER_H

#include "eventbus/EventBus.hpp"

class EventBusManager
{
public:
    static EventBusManager& instance()
    {
        static EventBusManager instance;
        return instance;
    }

    EventBus& bus()
    {
        return event_bus;
    }

private:
    EventBusManager()
    {
        EventBus::EventBusConfig config;
        config.task_max = 1024;
        config.task_model = EventBus::TaskModel::NORMAL;
        config.thread_max = 8;
        config.thread_min = 2;
        config.thread_model = EventBus::ThreadModel::DYNAMIC;

        event_bus.initEventBus(config);
    }

    EventBusManager(const EventBusManager&) = delete;
    EventBusManager& operator=(const EventBusManager&) = delete;
    EventBusManager(EventBusManager&&) = delete;
    EventBusManager& operator=(EventBusManager&&) = delete;

    EventBus event_bus;
};

#endif
