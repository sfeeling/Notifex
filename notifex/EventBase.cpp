//
// Created by sfeeling on 18-11-18.
//

#include "EventBase.h"

#include <memory.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <unistd.h>
#include <memory>
#include <iostream>
#include <utility>
#include <vector>

namespace notifex
{


EventBase::EventBase()
    :   debug_mode_(false),
        demultiplexer_(std::make_unique<Epoller>())
{
}

void EventBase::AddEvent(const Event &event)
{
    event_q_[event.fd_] = std::make_shared<Event>(event);
    demultiplexer_->RegisterEvent(event);
}

void EventBase::AddTimer(const Timer &timer)
{
    timer_q_.push(std::make_shared<Timer>(timer));
    if (debug_mode_)
    {
        std::cout << "DEBUG AddTimer GetTime: " << timer_q_.top()->GetTriggeringTime() << std::endl;
    }

}

void EventBase::SetUpTimer()
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
        if (debug_mode_)
        {
            std::cout << "DEBUG Initialize Timer: " << ptr->GetTriggeringTime() << std::endl;
        }
        timer_q_.push(ptr);
    }
}

void EventBase::Dispatch()
{
    // 根据Timer的时间间隔设置触发时间戳
    SetUpTimer();

    const int EPOLL_LIST_SIZE = 10;
    epoll_event epoll_list[EPOLL_LIST_SIZE];

    timeval time_stamp = { 0, 0 };
    bool done = false;
    while (!done)
    {
        if (event_q_.empty() && timer_q_.empty())
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
                std::shared_ptr<Timer> ptr = timer_q_.top();
                timer_q_.pop();
                if (debug_mode_)
                {
                    std::cout << "Trigger before epoll: " << ptr->GetTriggeringTime() << std::endl;
                }

                ptr->Trigger();
                // 如果Timer不是一次性的，则重新设置时间戳并添加到队列中
                if (!ptr->Once())
                {
                    timer_q_.push(ptr);
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

        if (debug_mode_)
            std::cout << "DEBUG timeout: " << timeout << std::endl;

        // IO复用,即调用epoll
        std::vector<int> active_list = demultiplexer_->GetActiveList(timeout);

        // 处理每个计时器事件
        if (!timer_q_.empty())
        {
            gettimeofday(&time_stamp, nullptr);
            msec_stamp = time_stamp.tv_sec * 1000 + time_stamp.tv_usec / 1000;
            while (!timer_q_.empty() && msec_stamp >= timer_q_.top()->GetTriggeringTime())
            {
                std::shared_ptr<Timer> ptr = timer_q_.top();
                timer_q_.pop();
                if (debug_mode_)
                {
                    std::cout << "DEBUG Trigger after epoll: " << ptr->GetTriggeringTime() << std::endl;
                }

                ptr->Trigger();
                if (!ptr->Once())
                {
                    timer_q_.push(ptr);
                }
            }
        }
        // 处理每个事件
        for (auto fd : active_list)
        {
            if (debug_mode_)
            {
                std::cout << "DEBUG fd from active_list: " << fd << std::endl;
            }

            std::shared_ptr<Event> ev_ptr = event_q_[fd];
            ev_ptr->Trigger();
        }
    }
}




}   // namespace notifex