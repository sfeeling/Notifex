//
// Created by sfeeling on 18-11-18.
//

#ifndef NOTIFEX_DEMULTIPLEXER_H
#define NOTIFEX_DEMULTIPLEXER_H

#include <vector>
#include "Event.h"

namespace notifex
{

// Demultiplexer(也可以叫做DEMUX)
class Demultiplexer
{
public:
    Demultiplexer() = default;
    virtual ~Demultiplexer() = default;

    virtual void RegisterEvent(const Event &) = 0;
    virtual void RemoveEvent() = 0;

    // Demultiplex
    virtual std::vector<int> GetActiveList() = 0;
};


}   // namespace notifex

#endif // NOTIFEX_DEMULTIPLEXER_H
