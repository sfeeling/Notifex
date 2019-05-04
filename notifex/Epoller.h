//
// Created by sfeeling on 18-11-18.
//

#ifndef NOTIFEX_EPOLLER_H
#define NOTIFEX_EPOLLER_H

#include <sys/epoll.h>

#include <mutex>
#include <vector>

#include "Demultiplexer.h"
#include "EventBase.h"

struct epoll_event;

namespace notifex
{

class Epoller : public Demultiplexer
{
public:
    Epoller();

    explicit Epoller(EventBase *event_base);
    ~Epoller() override;

    void RegisterEvent(const Event &) override;
    void RemoveEvent(const int &fd) override;
    void RegisterListener(const TCPListener &) override;

    // Demultiplex
    std::vector<int> GetActiveList(const int &msec) override;

    void Poll(int timeout_ms, ChannelList *active_channels) override;
    void UpdateChannel(Channel *channel) override;
    void RemoveChannel(Channel *channel) override;

private:
    int ep_fd_;
    const int ACTIVE_SIZE = 10;
    epoll_event event_, event_list_[10];

    static const int kInitEventListSize = 16;

    static const char *OperationToString(int op);

    void FillActiveChannels(int num_events, ChannelList *active_channels);
    void Update(int operation, Channel *channel);

    typedef std::vector<struct epoll_event> EventList;

    int epoll_fd_;
    EventList events_;
    std::mutex mutex_;
};


}   // namespace notifex


#endif // NOTIFEX_EPOLLER_H
