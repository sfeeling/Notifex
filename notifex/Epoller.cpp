//
// Created by sfeeling on 18-11-18.
//

#include "Epoller.h"

#include <cassert>
#include <iostream>

void notifex::Epoller::RegisterEvent(const Event &event)
{
    epoll_event ep_event = { 0, { nullptr } };
    // TODO: 考虑写事件
    if (event.EventIn())
        ep_event.events |= EPOLLIN;
    if (event.EventOut())
        ep_event.events |= EPOLLOUT;
    ep_event.data.fd = event.fd_;
    if (epoll_ctl(ep_fd_, EPOLL_CTL_ADD, event.fd_, &ep_event) == -1)
    {
        std::cerr << "epoll_ctl_add error" << std::endl;
        exit(EXIT_FAILURE);
    }

}

void notifex::Epoller::RemoveEvent(const int &fd)
{
    if (epoll_ctl(ep_fd_, EPOLL_CTL_DEL, fd, &event_) == -1)
    {
        std::cerr << "EPOLL_CTL_DEL error" << std::endl;
        exit(EXIT_FAILURE);
    }
}

std::vector<int> notifex::Epoller::GetActiveList(const int &msec)
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

notifex::Epoller::Epoller()
    :   ep_fd_(epoll_create1(0))
{

}

void notifex::Epoller::RegisterListener(const notifex::TCPListener &listener)
{
    epoll_event ep_event = { 0, { nullptr } };
    ep_event.events |= EPOLLIN;

    // TODO: 应该封装listen_fd_
    ep_event.data.fd = listener.listen_fd_;
    if (epoll_ctl(ep_fd_, EPOLL_CTL_ADD, listener.listen_fd_, &ep_event) == -1)
    {
        std::cerr << "epoll_ctl_add error" << std::endl;
        exit(EXIT_FAILURE);
    }

}
