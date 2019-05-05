//
// Created by sfeeling on 19-4-22.
//

#include "Listener.h"


#include <fcntl.h>
#include <unistd.h>

#include <cassert>
#include <cstring>

#include <glog/logging.h>

using namespace notifex;


int Listener::Accept()
{
    socklen_t cli_len = sizeof(cli_addr_);
    int con_fd = accept(listen_fd_, (sockaddr *)&cli_addr_, &cli_len);
}

Listener::Listener(EventBase *event_base, const SockAddress &listen_addr)
        :   event_base_(event_base),
            accept_socket_(Socket::CreateNonblockingOrDie(AF_INET)),
            accept_channel_(event_base, accept_socket_.Fd()),
            listenning_(false),
            idle_fd_(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{
    assert(idle_fd_ >= 0);
    accept_socket_.SetReuseAddr(true);


    // TODO: 需要重构
    accept_socket_.Bind(listen_addr);
    accept_channel_.SetReadCallback(
            std::bind(&Listener::HandleRead, this));
}

Listener::~Listener()
{
    accept_channel_.DisableAll();
    accept_channel_.Remove();
    ::close(idle_fd_);
}

void Listener::HandleRead()
{
    sockaddr_in cli_addr;
    // TODO:: 需要重构

    SockAddress peer_addr;
    int conn_fd = accept_socket_.Accept(&peer_addr);

    if (conn_fd >= 0)
    {
        if (new_connection_callback_)
        {
            new_connection_callback_(conn_fd, peer_addr);
        }
        else
        {
            if (::close(conn_fd) < 0)
            {
                LOG(ERROR) << "Listener::HandleRead ::close";
            }
        }
    }
    else
    {
        LOG(ERROR) << "Listener::HandleRead";

        if (errno == EMFILE)
        {
            ::close(idle_fd_);
            idle_fd_ = ::accept(accept_socket_.Fd(), nullptr, nullptr);
            ::close(idle_fd_);
            idle_fd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}

void Listener::Listen()
{
    listenning_ = true;
    accept_socket_.Listen();
    accept_channel_.EnableReading();
}



