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
#include <mutex>
#include <queue>
#include <unordered_map>
#include <vector>


#include "Callbacks.h"
#include "Listener.h"
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

    void AddTimer(const Timer &timer);
    //void AddSignal();

    void Dispatch();
    void NewDispatch();
    void Quit();


    void QueueInBase(Functor cb);
    void UpdateChannel(Channel *channel);
    void RemoveChannel(Channel *channel);
    bool HasChannel(Channel *channel);

    bool EventHandling() const { return event_handling_; }

    ThreadPool *GetThreadPool() { return &thread_pool_; };

private:
    void StartUpTimer();
    void HandleEvents(const std::vector<int> &active_list);

private:
    void HandleRead();
    void DoPendingFunctors();

    void PrintActiveChannels() const;

    // 线程池
    ThreadPool thread_pool_;

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
    std::shared_ptr<Listener> listener_;

    typedef std::vector<Channel *> ChannelList;

    bool dispatching_;
    std::atomic<bool> done_;
    std::atomic<bool> event_handling_;

    bool calling_pending_functors_;
    // 复用器
    std::unique_ptr<Demultiplexer> demultiplexer_;
    int64_t iteration_;
    // 活动通道
    ChannelList active_channels_;
    Channel *current_active_channel_;

#if defined(__clang__) && (!defined(SWIG))
#define THREAD_ANNOTATION_ATTRIBUTE__(x)   __attribute__((x))
#else
#define THREAD_ANNOTATION_ATTRIBUTE__(x)   // no-op
#endif

#define GUARDED_BY(x) \
    THREAD_ANNOTATION_ATTRIBUTE__(guarded_by(x))

    mutable std::mutex mutex_;
    std::vector<Functor> pending_functors_ GUARDED_BY(mutex_);
};


}   // namespace notifex


#endif // NOTIFEX_EVENTBASE_H
