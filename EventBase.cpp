//
// Created by sfeeling on 18-11-18.
//

#include "EventBase.h"

#include <memory.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <memory>
#include <iostream>
#include <utility>
#include <vector>

namespace notifex
{

EventBase::EventBase()
{

}

void EventBase::AddEvent(Event &event)
{
    event_q_[event.fd_] = std::shared_ptr<Event>(&event);
    std::cout << event_q_[event.fd_]->fd_ << std::endl;
    demultiplexer_->RegisterEvent(event);
}

void EventBase::Dispatch()
{
    const int EPOLL_LIST_SIZE = 10;
    epoll_event epoll_list[EPOLL_LIST_SIZE];

    bool done = false;
    while (!done)
    {
        // IO复用
        std::vector<int> active_list = demultiplexer_->GetActiveList();

        // 处理每个事件
        for (auto fd : active_list)
        {
            std::cout << "fd from active_list: " << fd << std::endl;
            std::shared_ptr<Event> ev_ptr = event_q_[fd];
            (*ev_ptr->callback_)(ev_ptr->fd_, ev_ptr->res_, ev_ptr->arg_);
        }
    }
}

}   // namespace notifex