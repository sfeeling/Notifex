//
// Created by sfeeling on 18-11-18.
//

#include "Event.h"

#include <fcntl.h>
#include <memory.h>

notifex::Event::Event(int fd, void (*callback)(int, int, void *))
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
    fcntl(fd_, F_SETFL, flags | O_NONBLOCK);
}
