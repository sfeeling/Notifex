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
    Timer(const Timer &);

    void SetTriggeringTime(const int &sec, const int &msec);
    void SetTriggeringTime(const long long int &msec);
    void SetTriggeringTime(const timeval *);
    long long GetTriggeringTime();

    void Trigger();
    inline bool Once();
    inline void SetOnce();
    inline void SetRepeated();

private:
    bool debug_mode_;

    timeval interval_;  // 时间间隔
    timeval triggering_time_;   // 触发时间
    timeval last_time_; // 上次触发时间
    bool ev_once_;

    void (*callback_)();
};

inline bool Timer::Once()
{
    return ev_once_;
}

inline void Timer::SetOnce()
{
    ev_once_ = true;
}

inline void Timer::SetRepeated()
{
    ev_once_ = false;
}

}   // namespace notifex

#endif // NOTIFEX_TIMER_H
