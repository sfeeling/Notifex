//
// Created by sfeeling on 18-11-18.
//

#ifndef NOTIFEX_EVENTBASE_H
#define NOTIFEX_EVENTBASE_H

#include <sys/epoll.h>
#include <ctime>
#include <memory>
#include <queue>
#include <unordered_map>
#include "Epoller.h"
#include "Event.h"
#include "Timer.h"

namespace notifex
{

class EventBase
{

public:
    EventBase();
    ~EventBase() = default;

    void AddEvent(Event &event);
    void AddTimer(Timer &timer);
    void Dispatch();

private:
    std::unordered_map<int, std::shared_ptr<Event>> event_q_;

    // Timer队列的比较器
    struct cmp
    {
        bool operator()(const std::shared_ptr<Timer> &pa, const std::shared_ptr<Timer> &pb) const
        {
            return pa->GetTriggeringTime() > pb->GetTriggeringTime();   // 以毫秒计
        }
    };
    // Timer队列
    std::priority_queue<std::shared_ptr<Timer>, std::vector<std::shared_ptr<Timer>>, cmp> timer_q_;

    std::unique_ptr<Demultiplexer> demultiplexer_;

};


}   // namespace notifex


#endif // NOTIFEX_EVENTBASE_H
