//
// Created by sfeeling on 18-11-18.
//

#include "EventBase.h"

#include <memory.h>
#include <signal.h>
#include <sys/eventfd.h>
#include <sys/time.h>
#include <unistd.h>

#include <functional>
#include <memory>
#include <iostream>
#include <utility>
#include <vector>

#include "Channel.h"
#include "Demultiplexer.h"

#include <glog/logging.h>

using namespace notifex;

namespace
{

const int kPollTimeMs = 10000;

int CreateEventFd()
{
    int event_fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (event_fd < 0)
    {
        LOG(ERROR) << "Failed in event fd";
        abort();
    }
    return event_fd;
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
class IgnoreSigPipe
{
public:
    IgnoreSigPipe()
    {
        ::signal(SIGPIPE, SIG_IGN);
        // LOG_TRACE << "Ignore SIGPIPE";
    }
};
#pragma GCC diagnostic error "-Wold-style-cast"

IgnoreSigPipe initObj;

}   // namespace



EventBase::EventBase()
    :   thread_pool_(8),
        // FIXME: demultiplexer_(std::make_unique<Epoller>()),
        demultiplexer_(Demultiplexer::NewDefaultDemultiplexer(this)),
        listener_(nullptr),
        dispatching_(false),
        done_(false),
        event_handling_(false),
        calling_pending_functors_(false),
        iteration_(0),
        wakeup_fd_(CreateEventFd()),
        wakeup_channel_(new Channel(this, wakeup_fd_)),
        current_active_channel_(nullptr)
{
    LOG(INFO) << "EventBase Created " << this;
    // FIXME: 参数绑定显示错误
    wakeup_channel_->SetReadCallback(
            std::bind(&EventBase::HandleRead, this));
    wakeup_channel_->EnableReading();
}

EventBase::~EventBase()
{
    LOG(INFO) << "EventBase::~EventBase";
    wakeup_channel_->DisableAll();
    wakeup_channel_->Remove();
    ::close(wakeup_fd_);
}

void EventBase::AddEvent(const Event &event)
{
    event_hash_[event.fd_] = std::make_shared<Event>(event);
    demultiplexer_->RegisterEvent(event);
}

void EventBase::AddEvent(const std::shared_ptr<Event> &ev_ptr)
{
    // TODO: 暂未测试
    event_hash_[ev_ptr->fd_] = ev_ptr;
    demultiplexer_->RegisterEvent(*ev_ptr);
    LOG(WARNING) << "ADDEVENT";
}

void EventBase::AddTimer(const Timer &timer)
{
    timer_q_.push(std::make_shared<Timer>(timer));
    LOG(INFO) << "AddTimer GetTime: " << timer_q_.top()->GetTriggeringTime();

}

void EventBase::AddListener(const TCPListener &listener)
{
    listener_ = std::make_shared<TCPListener>(listener);
    demultiplexer_->RegisterListener(listener);
    LOG(INFO) << "AddTCPListener" << "listen fd:" << listener_->GetListenFd();
}

void EventBase::StartUpTimer()
{
    long tq_size = timer_q_.size();
    timeval time_stamp = { 0, 0 };
    gettimeofday(&time_stamp, nullptr);
    for (int i = 0; i < tq_size; ++i)
    {
        // 取出未设置触发时间的Timer，设置好后放回队列中
        std::shared_ptr<Timer> ptr = timer_q_.top();
        timer_q_.pop();
        ptr->SetTriggeringTime(&time_stamp);
        LOG(INFO) << "Initialize Timer: " << ptr->GetTriggeringTime();
        timer_q_.push(ptr);
    }
}

void EventBase::Dispatch()
{
    // 根据Timer的时间间隔设置触发时间戳
    StartUpTimer();

    const int EPOLL_LIST_SIZE = 10;
    epoll_event epoll_list[EPOLL_LIST_SIZE];

    timeval time_stamp = { 0, 0 };
    bool done = false;
    while (!done)
    {
        if (event_hash_.empty() && timer_q_.empty())
            done = true;

        // 如果没有计时器事件，timeout为-1让epoll永久阻塞
        // TODO: 后续将更改timeout设置为可选
        int timeout = -1;
        gettimeofday(&time_stamp, nullptr);
        long long msec_stamp = time_stamp.tv_sec * 1000 + time_stamp.tv_usec / 1000;

        // 在epoll之前处理每个计时器事件（如果有的话）
        // 一般来说，Timer事件的优先级比较高，需要随时进行处理
        if (!timer_q_.empty())
        {
            while (!timer_q_.empty() && msec_stamp >= timer_q_.top()->GetTriggeringTime())
            {
                std::shared_ptr<Timer> tm_ptr = timer_q_.top();
                timer_q_.pop();

                LOG(INFO) << "Trigger before epoll: " << tm_ptr->GetTriggeringTime();


                // TODO: Test Thread Pool with Member Function
                // 这里相当于function<void()> f = std::bind(&Timer::Trigger, tm_ptr)
                // f被传递给线程池，当绑定到非静态成员函数(Timer::Trigger)时，必须要有实例对其进行调用
                // 即用于执行函数的是ptr所指向的实例,由于每个实例的内部参数不同，调用才得以实现
                thread_pool_.execute(std::bind(&Timer::Trigger, tm_ptr));
                //tm_ptr->Trigger();

                // TODO: 如果要换用线程模式，必须对Timer实例上锁，否则repeated模式中任务未执行完的情况下，
                // TODO: Timer又重新添加到队列中，则队列中Timer的触发时间仍未更新，
                // TODO: 在之前的任务未结束之前，又多了几个同时刻的新任务，产生严重错误
                // 如果Timer不是一次性的，则重新设置时间戳并添加到队列中
                if (!tm_ptr->Once())
                {
                    // 应该在添加到队列之前设置下次触发时间
                    // 之前把设置触发时间的逻辑放在了Timer内部，回调之后，在未执行完回调时，
                    // 由于是多线程，这里可能已经把未更新的Timer push到队列中，导致重复运行
                    tm_ptr->SetNextTime();
                    timer_q_.push(tm_ptr);
                }

            }

            // TODO 处理timeout的类型转换,用memcpy可能会出错
            // TODO 这里是否要重新获取msec_stamp？
            if (!timer_q_.empty() && msec_stamp < timer_q_.top()->GetTriggeringTime())
            {
                long long temp = timer_q_.top()->GetTriggeringTime() - msec_stamp;
                memcpy(&timeout, &temp, sizeof(timeout));
            }
        }

        LOG(INFO) << "Timeout: " << timeout;

        // IO复用,即调用epoll
        std::vector<int> active_list = demultiplexer_->GetActiveList(timeout);

        // 处理每个计时器事件
        if (!timer_q_.empty())
        {
            gettimeofday(&time_stamp, nullptr);
            msec_stamp = time_stamp.tv_sec * 1000 + time_stamp.tv_usec / 1000;
            while (!timer_q_.empty() && msec_stamp >= timer_q_.top()->GetTriggeringTime())
            {
                std::shared_ptr<Timer> tm_ptr = timer_q_.top();
                timer_q_.pop();

                LOG(INFO) << "DEBUG Trigger after epoll: " << tm_ptr->GetTriggeringTime();


                // TODO: Test Thread Pool with Member Function
                thread_pool_.execute(std::bind(&Timer::Trigger, tm_ptr));
                // tm_ptr->Trigger();
                if (!tm_ptr->Once())
                {
                    tm_ptr->SetNextTime();
                    timer_q_.push(tm_ptr);
                }
            }
        }

        HandleEvents(active_list);
        // 处理每个事件

    }
}

void EventBase::HandleEvents(const std::vector<int> &active_list)
{
    //
    for (auto fd : active_list)
    {

        LOG(INFO) << "fd from active_list: " << fd;
        if (listener_ && listener_->GetListenFd() == fd)
        {
            // TODO: 创建连接事件
            int con_fd = listener_->Accept();
            std::shared_ptr<Event> ev_ptr = std::make_shared<Event>(con_fd, listener_->callback_);
            AddEvent(ev_ptr);
            LOG(WARNING) << "创建连接: " << fd;
        }
        else
        {
            std::shared_ptr<Event> ev_ptr = event_hash_[fd];
            // ev_ptr->Trigger();
            thread_pool_.execute(std::bind(&Event::Trigger, ev_ptr));
        }


    }
}

void EventBase::Quit()
{
    done_ = true;
}

void EventBase::Run(EventBase::Functor cb)
{

}

void EventBase::Wakeup()
{

}

void EventBase::UpdateChannel(Channel *channel)
{
    demultiplexer_->UpdateChannel(channel);
}

void EventBase::RemoveChannel(Channel *channel)
{
    if (event_handling_)
    {
        // FIXME: assert(current_active_channel_ == channel ||
        //              std::find(active_channels_.begin(), active_channels_.end(), channel) == active_channels_.end());
        // 在活动的通道中应该找不到该通道
        assert(current_active_channel_ == channel);
    }
    demultiplexer_->RemoveChannel(channel);
}

bool EventBase::HasChannel(Channel *channel)
{
    return demultiplexer_->HasChannel(channel);
}

void EventBase::HandleRead()
{
    uint64_t one = 1;
    ssize_t n = ::read(wakeup_fd_, &one, sizeof(one));
    if (n != sizeof(one))
    {
        LOG(ERROR) << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
}

void EventBase::DoPendingFunctors()
{

}

void EventBase::NewDispatch()
{
    assert(!dispatching_);
    dispatching_ = true;
    done_ = false;
    LOG(INFO) << "EventBase " << " start dispatching";

    while (!done_)
    {
        active_channels_.clear();
        demultiplexer_->Poll(kPollTimeMs, &active_channels_);
        ++iteration_;
        // TODO: TEST
        if (false)
            PrintActiveChannels();
        // TODO sort channel by priority
        event_handling_ = true;
        for (auto channel : active_channels_)
        {
            current_active_channel_ = channel;
            // TODO: 多线程 thread_pool_.execute(std::bind(&Channel::HandleEvent, current_active_channel_));
            current_active_channel_->HandleEvent();
        }
        current_active_channel_ = nullptr;  // NULL
        event_handling_ = false;
        // 这里没搞懂
        DoPendingFunctors();
    }

    LOG(INFO) << "EventBase " << " stop dispatching";
    dispatching_ = false;
}

void EventBase::PrintActiveChannels() const
{
    for (const auto channel : active_channels_)
    {
        LOG(INFO) << "{" << channel->ReventsToString() << "} ";
    }
}

