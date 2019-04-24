//
// Created by sfeeling on 19-4-24.
//

#include "Channel.h"

#include <poll.h>

#include <sstream>

#include "EventBase.h"

#include <glog/logging.h>

using namespace notifex;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventBase *event_base, int fd)
    :   event_base_(event_base),
        fd_(fd),
        events_(0),
        revents_(0),
        index_(-1),
        event_handling_(false),
        added_to_base_(false)
{

}

Channel::~Channel()
{
    assert(!event_handling_);
    assert(!added_to_base_);
    assert(!event_base_->HasChannel(this));
}

void Channel::HandleEvent()
{
    // FIXME: SAFELY
    HandleEventSafely();
}

std::string Channel::EventsToString(int fd, int ev)
{
    std::ostringstream oss;
    oss << fd << ": ";
    if (ev & POLLIN)
        oss << "IN ";
    if (ev & POLLPRI)
        oss << "PRI ";
    if (ev & POLLOUT)
        oss << "OUT ";
    if (ev & POLLHUP)
        oss << "HUP ";
    if (ev & POLLRDHUP)
        oss << "RDHUP ";
    if (ev & POLLERR)
        oss << "ERR ";
    if (ev & POLLNVAL)
        oss << "NVAL ";

    return oss.str();
}

void Channel::Update()
{
    added_to_base_ = true;
    event_base_->UpdateChannel(this);
}

void Channel::HandleEventSafely()
{
    // FIXME: 在这里使用线程池，或者外部使用
    event_handling_ = true;
    LOG(INFO) << "Channel::HandleEventSafely" << ReventsToString();
    if ((revents_ & POLLHUP) && !(revents_ & POLLIN))
    {

        if (close_callback_) close_callback_();
    }

    if (revents_ & POLLNVAL)
    {
        LOG(WARNING) << "fd = " << fd_ << " Channel::handle_event() POLLNVAL";
    }

    if (revents_ & (POLLERR | POLLNVAL))
    {
        if (error_callback_) error_callback_();
    }
    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP))
    {
        if (read_callback_) read_callback_();
    }
    if (revents_ & POLLOUT)
    {
        if (write_callback_) write_callback_();
    }
    event_handling_ = false;
}

void Channel::Remove()
{
    assert(IsNoneEvent());
    added_to_base_ = false;
    event_base_->RemoveChannel(this);
}

std::string Channel::EventsToString() const
{
    return EventsToString(fd_, events_);
}

std::string Channel::ReventsToString() const
{
    return EventsToString(fd_, revents_);
}
