//
// Created by sfeeling on 18-11-19.
//


#include "Timer.h"

#include <memory.h>
#include <iostream>


notifex::Timer::Timer(int sec, int msec, void (*callback)())
    :   interval_({sec, msec * 1000}),   // 秒和微秒
        triggering_time_({0, 0}),
        callback_(callback),
        ev_once_(true),
        debug_mode_(false)
{

}

notifex::Timer::Timer(const notifex::Timer &timer)
{
    memcpy(&interval_, &timer.interval_, sizeof(timeval));
    memcpy(&triggering_time_, &timer.triggering_time_, sizeof(timeval));
    memcpy(&last_time_, &timer.last_time_, sizeof(timeval));
    ev_once_ = timer.ev_once_;
    callback_ = timer.callback_;
    debug_mode_ = timer.debug_mode_;
}

void notifex::Timer::SetTriggeringTime(const int &sec, const int &msec)
{
    triggering_time_.tv_sec = sec;
    triggering_time_.tv_usec = msec * 1000;
}

long long notifex::Timer::GetTriggeringTime()
{
    return triggering_time_.tv_sec * 1000 + triggering_time_.tv_usec / 1000;
}

void notifex::Timer::SetTriggeringTime(const long long int &msec)
{
    triggering_time_.tv_sec = msec / 1000;
    triggering_time_.tv_usec = (msec % 1000) * 1000;
}

void notifex::Timer::SetTriggeringTime(const timeval *time_stamp)
{
    triggering_time_.tv_sec = time_stamp->tv_sec + interval_.tv_sec;
    triggering_time_.tv_usec = time_stamp->tv_usec + interval_.tv_usec;
    if (triggering_time_.tv_usec >= 1000000)
    {
        triggering_time_.tv_usec -= 1000000;
        triggering_time_.tv_sec += 1;
    }
}

void notifex::Timer::Trigger()
{
    // 触发回调函数
    if (debug_mode_)
        std::cout << "Trigger()内部" << std::endl;
    (*callback_)();
    triggering_time_.tv_sec += interval_.tv_sec;
    triggering_time_.tv_usec += interval_.tv_usec;
    if (triggering_time_.tv_usec >= 1000000)
    {
        triggering_time_.tv_usec -= 1000000;
        triggering_time_.tv_sec += 1;
    }
}



