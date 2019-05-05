//
// Created by sfeeling on 18-11-18.
//

#include "Epoller.h"

#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <cassert>
#include <iostream>

#include "Channel.h"

#include <glog/logging.h>

using namespace notifex;

// On Linux, the constants of poll(2) and epoll(4)
// are expected to be the same.
static_assert(EPOLLIN == POLLIN,        "epoll uses same flag values as poll");
static_assert(EPOLLPRI == POLLPRI,      "epoll uses same flag values as poll");
static_assert(EPOLLOUT == POLLOUT,      "epoll uses same flag values as poll");
static_assert(EPOLLRDHUP == POLLRDHUP,  "epoll uses same flag values as poll");
static_assert(EPOLLERR == POLLERR,      "epoll uses same flag values as poll");
static_assert(EPOLLHUP == POLLHUP,      "epoll uses same flag values as poll");


namespace
{
    const int kNew = -1;
    const int kAdded = 1;
    const int kDeleted = 2;
}


std::vector<int> Epoller::GetActiveList(const int &msec)
{
    std::vector<int> active_list;
    // TODO ACTIVE_SIZE应为最大的fd数，而不是返回数
    int n_fds = epoll_wait(ep_fd_, event_list_, ACTIVE_SIZE, msec);
    if (n_fds == 0)
        return active_list;
    if (n_fds < 0)
    {
        std::cerr << "epoll_wait error" << std::endl;
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < n_fds; ++i)
        active_list.push_back(event_list_[i].data.fd);

    return active_list;
}

Epoller::Epoller()
    :   Demultiplexer(nullptr),
        ep_fd_(epoll_create1(0))
{

}

Epoller::Epoller(EventBase *event_base)
    :   Demultiplexer(event_base),
        epoll_fd_(::epoll_create1(EPOLL_CLOEXEC)),
        events_(kInitEventListSize),
        ep_fd_(0)   // TODO: 后续删除
{
    if (epoll_fd_ < 0)
    {
        LOG(FATAL) << "Epoller::Epoller";
    }
}

Epoller::~Epoller()
{
    ::close(epoll_fd_);
}

const char *Epoller::OperationToString(int op)
{
    switch (op)
    {
        case EPOLL_CTL_ADD:
            return "ADD";
        case EPOLL_CTL_DEL:
            return "DEL";
        case EPOLL_CTL_MOD:
            return "MOD";
        default:
            // FIXME: 原代码assert(false && "ERROR op");
            assert(false);

            return "Unknown Operation";
    }
}

void Epoller::FillActiveChannels(int num_events, ChannelList *active_channels)
{
    assert(implicit_cast<size_t>(num_events) <= events_.size());
    for (int i = 0; i < num_events; ++i)
    {
        auto channel = static_cast<Channel *>(events_[i].data.ptr);
#ifndef NDEBUG
        int fd = channel->Fd();
        ChannelMap::const_iterator iter = channels_.find(fd);
        assert(iter != channels_.end());
        assert(iter->second == channel);
#endif

        channel->SetRevents(events_[i].events);
        active_channels->push_back(channel);
    }
}

void Epoller::Update(int operation, Channel *channel)
{
    struct epoll_event event;
    memZero(&event, sizeof event);
    event.events = channel->Events();
    event.data.ptr = channel;
    int fd = channel->Fd();
    LOG(INFO) << "epoll_ctl op = " << OperationToString(operation)
              << " fd = " << fd << " event = { " << channel->EventsToString() << " }";
    if (::epoll_ctl(epoll_fd_, operation, fd, &event) < 0)
    {
        if (operation == EPOLL_CTL_DEL)
        {
            LOG(ERROR) << "epoll_ctl op =" << OperationToString(operation) << " fd =" << fd;
        }
        else
        {
            LOG(FATAL) << "epoll_ctl op =" << OperationToString(operation) << " fd =" << fd;
        }
    }
}

void Epoller::Poll(int timeout_ms, ChannelList *active_channels)
{
    // FIXME: 同步
    LOG(INFO) << "fd total count " << channels_.size();
    int num_events = ::epoll_wait(epoll_fd_,
                                 &*events_.begin(),
                                 static_cast<int>(events_.size()),
                                 timeout_ms);
    int saved_errno = errno;
    if (num_events > 0)
    {
        LOG(INFO) << num_events << " events happened";
        FillActiveChannels(num_events, active_channels);
        if (implicit_cast<size_t>(num_events) == events_.size())
        {
            events_.resize(events_.size() * 2);
        }
    }
    else if (num_events == 0)
    {
        LOG(INFO) << "nothing happened";
    }
    else
    {
        // error happens, log uncommon ones
        if (saved_errno != EINTR)
        {
            errno = saved_errno;
            LOG(INFO) << "EPoller::Poll()";
        }
    }
    // return now;
}

void Epoller::UpdateChannel(Channel *channel)
{
    std::lock_guard<std::mutex> lck(mutex_);
    const int index = channel->Index();
    LOG(INFO) << "fd = " << channel->Fd()
              << " events = " << channel->Events() << " index = " << index;
    if (index == kNew || index == kDeleted)
    {
        // a new one, add with EPOLL_CTL_ADD
        int fd = channel->Fd();
        if (index == kNew)
        {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        }
        else // index == kDeleted
        {
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }

        channel->SetIndex(kAdded);
        Update(EPOLL_CTL_ADD, channel);
    }
    else
    {
        // update existing one with EPOLL_CTL_MOD/DEL
        int fd = channel->Fd();
        (void)fd;
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(index == kAdded);
        if (channel->IsNoneEvent())
        {
            Update(EPOLL_CTL_DEL, channel);
            channel->SetIndex(kDeleted);
        }
        else
        {
            Update(EPOLL_CTL_MOD, channel);
        }
    }
}

void Epoller::RemoveChannel(Channel *channel)
{
    std::lock_guard<std::mutex> lck(mutex_);
    int fd = channel->Fd();
    LOG(INFO) << "fd = " << fd;
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->IsNoneEvent());
    int index = channel->Index();
    assert(index == kAdded || index == kDeleted);


    size_t n = channels_.erase(fd);
    (void)n;
    assert(n == 1);




    if (index == kAdded)
    {
        Update(EPOLL_CTL_DEL, channel);
    }
    channel->SetIndex(kNew);
}


