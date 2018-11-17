#ifndef NOTIFEX_H
#define NOTIFEX_H

#include <queue>

namespace notifex
{

class EventBase
{
public:
    EventBase() = default;
    ~EventBase() = default;

    void CreateEvent();
    void AddEvent();
    void Dispatch();

private:
    std::queue<int> event_q_;

};


}   // namespace notifex


#endif