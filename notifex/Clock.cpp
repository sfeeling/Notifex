//
// Created by sfeeling on 19-1-8.
//

#include "Clock.h"

#include <sys/time.h>
#include <cstdlib>

void notifex::Clock::Tick(const int &index)
{
    timeval time_stamp = { 0, 0 };
    gettimeofday(&time_stamp, nullptr);
    tick_q_[index] = time_stamp;
}

// 返回的是毫秒
long long notifex::Clock::GetInterval(const int &ind1, const int &ind2)
{
    long long t1 = tick_q_[ind1].tv_sec * 1000 +tick_q_[ind1].tv_usec / 1000;
    long long t2 = tick_q_[ind2].tv_sec * 1000 +tick_q_[ind2].tv_usec / 1000;
    return llabs(t1 - t2);
}
