//
// Created by sfeeling on 18-11-18.
//

#ifndef NOTIFEX_DEMULTIPLEXER_H
#define NOTIFEX_DEMULTIPLEXER_H

#include <map>
#include <vector>


#include "Event.h"
#include "EventBase.h"
#include "TCPListener.h"

namespace notifex
{

class Channel;

// Demultiplexer(也可以叫做DEMUX) Poller
class Demultiplexer
{
public:
    typedef std::vector<Channel *> ChannelList;

    explicit Demultiplexer(EventBase *event_base);
    virtual ~Demultiplexer();

    // 原构造函数
    Demultiplexer() = default;


    // 原函数
    virtual void RegisterEvent(const Event &event) = 0;
    virtual void RemoveEvent(const int &fd) = 0;
    virtual void RegisterListener(const TCPListener &listener) = 0;

    // Demultiplex
    virtual std::vector<int> GetActiveList(const int &msec) = 0;

    // 返回的是active channels
    virtual void Poll(int timeout_ms, ChannelList *active_channels) = 0;

    virtual void UpdateChannel(Channel *channel) = 0;
    virtual void RemoveChannel(Channel *chanel) = 0;
    virtual bool HasChannel(Channel *channel) const;

    static Demultiplexer *NewDefaultDemultiplexer(EventBase *event_base);



protected:
    // TODO: 后续和unordered_map对比
    typedef std::map<int, Channel*> ChannelMap;
    // FIXME: 需要原子操作
    ChannelMap channels_;

private:
    EventBase *event_base_;
};


}   // namespace notifex

#endif // NOTIFEX_DEMULTIPLEXER_H
