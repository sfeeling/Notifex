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

    virtual void RegisterEvent(const Event &event) = 0;
    virtual void RemoveEvent(const int &fd) = 0;

    // Demultiplex
    virtual std::vector<int> GetActiveList(const int &msec) = 0;
};


}   // namespace notifex

#endif // NOTIFEX_DEMULTIPLEXER_H
