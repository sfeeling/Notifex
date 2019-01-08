//
// Created by sfeeling on 18-11-19.
//

#ifndef NOTIFEX_TIMER_H
#define NOTIFEX_TIMER_H

#include <sys/time.h>

namespace notifex
{

class Timer
{

public:
    // 精度暂时限定在毫秒
    Timer(int sec, int msec, void (*callback)());
    ~Timer() = default;
    Timer(const Timer &) = default;

    void SetTriggeringTime(const int &sec, const int &msec);
    void SetTriggeringTime(const long long int &msec);
    void SetTriggeringTime(const timeval *);
    long long GetTriggeringTime();

    void Trigger();

private:
    bool debug_mode_;

    timeval interval_;  // 时间间隔
    timeval triggering_time_;   // 触发时间
    timeval last_time_; // 上次触发时间
    bool ev_once_;

    void (*callback_)();
};

}   // namespace notifex

#endif // NOTIFEX_TIMER_H
