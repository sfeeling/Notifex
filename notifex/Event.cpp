//
// Created by sfeeling on 18-11-18.
//

#include "Event.h"

#include <fcntl.h>
#include <memory.h>

notifex::Event::Event(const int &fd, void (*callback)(int, int, void *))
    :   fd_(fd),
        ev_in_(true),
        ev_out_(false),
        nonblock_(true),
        res_(0),
        arg_(nullptr),
        callback_(callback)
{
    // 设置为非阻塞
    int flags = fcntl(fd_, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(fd_, F_SETFL, flags);
}

notifex::Event::Event(const Event &event)
{
    fd_ = event.fd_;
    ev_in_ = event.ev_in_;
    ev_out_ = event.ev_out_;
    nonblock_ = event.nonblock_;
    res_ = event.res_;
    arg_ = event.arg_;
    callback_ = event.callback_;
}