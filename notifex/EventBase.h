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
#include "TCPListener.h"
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
    void AddEvent(const std::shared_ptr<Event> &ev_ptr);
    void AddTimer(const Timer &timer);
    void AddListener(const TCPListener &listener);
    //void AddSignal();
    void Dispatch();

private:
    void StartUpTimer();
    void ProcessEvents(const std::vector<int> &active_list);

private:
    // 线程池
    ThreadPool thread_pool_;

    // 事件队列
    std::unordered_map<int, std::shared_ptr<Event>> event_hash_;

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

    // TCP监听器
    std::shared_ptr<TCPListener> listener_;
};


}   // namespace notifex


#endif // NOTIFEX_EVENTBASE_H
