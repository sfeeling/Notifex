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

void EventBase::AddEvent(Event &event)
{
    event_q_[event.fd_] = std::shared_ptr<Event>(&event);
    demultiplexer_->RegisterEvent(event);
}

void EventBase::AddTimer(Timer &timer)
{
    timer_q_.push(std::shared_ptr<Timer>(&timer));
    if (debug_mode_)
    {
        std::cout << "DEBUG AddTimer GetTime: " << timer_q_.top()->GetTriggeringTime() << std::endl;
    }

}

void EventBase::Dispatch()
{
    const int EPOLL_LIST_SIZE = 10;
    epoll_event epoll_list[EPOLL_LIST_SIZE];

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

    bool done = false;
    while (!done)
    {
        if (event_q_.empty() && timer_q_.empty())
            done = true;

        // 如果没有计时器事件，timeout为-1让epoll永久阻塞
        int timeout = -1;
        gettimeofday(&time_stamp, nullptr);
        long long msec_stamp = time_stamp.tv_sec * 1000 + time_stamp.tv_usec / 1000;
        // 在epoll之前处理每个计时器事件（如果有的话）
        if (!timer_q_.empty())
        {
            while (msec_stamp >= timer_q_.top()->GetTriggeringTime())
            {
                std::shared_ptr<Timer> ptr = timer_q_.top();
                timer_q_.pop();
                if (debug_mode_)
                {
                    std::cout << "Trigger before epoll: " << ptr->GetTriggeringTime() << std::endl;
                }

                ptr->Trigger();
                timer_q_.push(ptr);
            }

            // 如果while循环处理正确，if语句应该可以省掉
            // FIXME 处理timeout的类型转换
            if (msec_stamp < timer_q_.top()->GetTriggeringTime())
            {
                long long temp = timer_q_.top()->GetTriggeringTime() - msec_stamp;
                timeout = 0;
                while (temp)
                {
                    ++timeout;
                    --temp;
                }
            }
        }

        if (debug_mode_)
            std::cout << "DEBUG timeout: " << timeout << std::endl;
        // IO复用
        std::vector<int> active_list = demultiplexer_->GetActiveList(timeout);

        // 处理每个计时器事件
        if (true /*active_list.empty(),不管是否为空，应该都要处理*/ )
        {
            gettimeofday(&time_stamp, nullptr);
            msec_stamp = time_stamp.tv_sec * 1000 + time_stamp.tv_usec / 1000;
            while (msec_stamp >= timer_q_.top()->GetTriggeringTime())
            {
                std::shared_ptr<Timer> ptr = timer_q_.top();
                timer_q_.pop();
                if (debug_mode_)
                {
                    std::cout << "DEBUG Trigger after epoll: " << ptr->GetTriggeringTime() << std::endl;
                }

                ptr->Trigger();
                timer_q_.push(ptr);
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

void EventBase::Debug()
{
    debug_mode_ = true;
}


}   // namespace notifex