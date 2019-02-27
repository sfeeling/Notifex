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
#include "RIO.h"
#include "ThreadPool.h"
#include "Timer.h"
#include "Util.h"

namespace notifex
{


class EventBase
{

public:
    EventBase();
    ~EventBase() = default;

    void AddEvent(const Event &event);
    void AddTimer(const Timer &timer);
    //void AddSignal();
    void Dispatch();

private:
    void StartUpTimer();

private:
    // 线程池
    ThreadPool thread_pool_;

    // 事件队列
    std::unordered_map<int, std::shared_ptr<Event>> event_q_;

    // 复用器
    std::unique_ptr<Demultiplexer> demultiplexer_;

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

};


}   // namespace notifex


#endif // NOTIFEX_EVENTBASE_H
