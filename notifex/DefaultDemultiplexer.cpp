//
// Created by sfeeling on 19-4-24.
//

#include "Demultiplexer.h"
#include "Epoller.h"

#include <cstdlib>

using namespace notifex;

Demultiplexer *Demultiplexer::NewDefaultDemultiplexer(EventBase *event_base)
{
    // TODO: 这里默认为使用epoll，后续添加其他IO复用API
    return new Epoller(event_base);
}