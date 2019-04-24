//
// Created by sfeeling on 19-4-24.
//

#include "Demultiplexer.h"

#include "Channel.h"

using namespace notifex;

Demultiplexer::Demultiplexer(EventBase *event_base)
    :   event_base_(event_base)
{
}

bool Demultiplexer::HasChannel(Channel *channel) const
{
    // TODO: ChannelMap::const_iterator iter
    auto iter = channels_.find(channel->Fd());
    return iter != channels_.end() && iter->second == channel;
}

Demultiplexer::~Demultiplexer() = default;

