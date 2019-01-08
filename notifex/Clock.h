//
// Created by sfeeling on 19-1-8.
//

#ifndef NOTIFEX_CLOCK_H
#define NOTIFEX_CLOCK_H

#include <unordered_map>

namespace notifex
{

class Clock
{
public:
    Clock() = default;
    ~Clock() = default;

    void Tick(const int &);

    // 返回的是毫秒
    long long GetInterval(const int &, const int &);

private:
    std::unordered_map<int, timeval> tick_q_;

};

}   // namespace notifex

#endif // NOTIFEX_CLOCK_H
