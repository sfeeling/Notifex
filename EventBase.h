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

namespace notifex
{


struct TcpListener
{
    int fd_;
    int flag_;
    void (*call_back_)(int, int, void *);
};


class EventBase
{
public:
    EventBase();
    ~EventBase() = default;

    void AddEvent(Event &event);
    void Dispatch();

private:
    std::unordered_map<int, std::shared_ptr<Event>> event_q_;
    std::queue<int> timer_q_;
    std::unique_ptr<Demultiplexer> demultiplexer_ = std::make_unique<Epoller>();
    //Demultiplexer &demultiplexer_ = Epoller();

};


}   // namespace notifex


#endif // NOTIFEX_EVENTBASE_H
