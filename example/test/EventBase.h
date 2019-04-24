//
// Created by sfeeling on 18-11-18.
//

#ifndef NOTIFEX_EVENTBASE_H
#define NOTIFEX_EVENTBASE_H

#include <sys/epoll.h>
#include <atomic>
#include <ctime>
#include <functional>
#include <memory>
#include <queue>
#include <unordered_map>


#include "Callbacks.h"
#include "Event.h"
#include "RIO.h"
#include "TCPListener.h"
#include "ThreadPool.h"
#include "Timer.h"
#include "Util.h"

namespace notifex
{

class Channel;
class Demultiplexer;



class EventBase
{

public:
    typedef std::function<void()> Functor;

    EventBase();
    ~EventBase();

    void AddEvent(const Event &event);
    void AddEvent(const std::shared_ptr<Event> &ev_ptr);
    void AddTimer(const Timer &timer);
    void AddListener(const TCPListener &listener);
    //void AddSignal();

    void Dispatch();
    void NewDispatch();
    void Quit();

    int64_t Iteration() const { return iteration_; }

    void Run(Functor cb);

    void Wakeup();
    void UpdateChannel(Channel *channel);
    void RemoveChannel(Channel *channel);
    bool HasChannel(Channel *channel);

    bool EventHandling() const { return event_handling_; }

private:
    void StartUpTimer();
    void HandleEvents(const std::vector<int> &active_list);

private:
    void HandleRead();
    void DoPendingFunctors();

    // 线程池
    ThreadPool thread_pool_;

    // 事件队列
    std::unordered_map<int, std::shared_ptr<Event>> event_hash_;


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

    typedef std::vector<Channel *> ChannelList;

    bool dispatching_;
    std::atomic<bool> done_;
    bool event_handling_;
    bool calling_pending_functors_;
    int64_t iteration_;
    // 复用器
    std::unique_ptr<Demultiplexer> demultiplexer_;
    int wakeup_fd_;
    std::unique_ptr<Channel> wakeup_channel_;

    // 活动通道
    ChannelList active_channels_;
    Channel *current_active_channel_;

};


}   // namespace notifex


#endif // NOTIFEX_EVENTBASE_H
