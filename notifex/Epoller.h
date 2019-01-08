//
// Created by sfeeling on 18-11-18.
//

#ifndef NOTIFEX_EPOLLER_H
#define NOTIFEX_EPOLLER_H

#include <sys/epoll.h>
#include "Demultiplexer.h"

namespace notifex
{

class Epoller : public Demultiplexer
{
public:
    Epoller();
    ~Epoller() override = default;

    void RegisterEvent(const Event &) override;
    void RemoveEvent(const int &fd) override;

    // Demultiplex
    std::vector<int> GetActiveList(const int &msec) override;

private:
    int ep_fd_;
    const int ACTIVE_SIZE = 10;
    epoll_event event_, event_list_[10];

};


}


#endif // NOTIFEX_EPOLLER_H
