//
// Created by sfeeling on 19-4-22.
//

#include "TCPListener.h"


#include <fcntl.h>
#include <unistd.h>

#include <cassert>
#include <cstring>

#include <glog/logging.h>

using namespace notifex;


TCPListener::TCPListener(uint16_t port, void (*callback)(int, int, void *))
    :   listen_fd_(socket(AF_INET, SOCK_STREAM, 0)),
        callback_(callback),
        accept_socket_(-1),
        accept_channel_(event_base_, -1)
{
    memset(&serv_addr_, 0, sizeof(serv_addr_));
    serv_addr_.sin_family = AF_INET;
    serv_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr_.sin_port = htons(port);

    bind(listen_fd_, (sockaddr *)&serv_addr_, sizeof(serv_addr_));
    LOG(INFO) << "Bind Success";
    listen(listen_fd_, 24);
    LOG(INFO) << "Listen Success";
}

int TCPListener::Accept()
{
    socklen_t cli_len = sizeof(cli_addr_);
    int con_fd = accept(listen_fd_, (sockaddr *)&cli_addr_, &cli_len);
}

TCPListener::TCPListener(EventBase *event_base, const sockaddr *serv_addr)
    :   event_base_(event_base),
        accept_socket_(Socket::CreateNonblockingOrDie(AF_INET)),
        accept_channel_(event_base, accept_socket_.GetFd()),
        listenning_(false),
        idle_fd_(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{
    assert(idle_fd_ >= 0);
    accept_socket_.SetReuseAddr(true);


    // TODO: 需要重构
    accept_socket_.Bind(serv_addr);
    accept_channel_.SetReadCallback(
            std::bind(&TCPListener::HandleRead, this)
            );
}

TCPListener::~TCPListener()
{
    accept_channel_.DisableAll();
    accept_channel_.Remove();
    ::close(idle_fd_);
}

void TCPListener::HandleRead()
{
    sockaddr_in cli_addr;
    // TODO:: 需要重构
    int conn_fd = accept_socket_.Accept((sockaddr *)&cli_addr);

    if (conn_fd >= 0)
    {
        if (new_connection_callback_)
        {
            new_connection_callback_(conn_fd);
        }
        else
        {
            if (::close(conn_fd) < 0)
            {
                LOG(ERROR) << "TCPListener::HandleRead ::close";
            }
        }
    }
    else
    {
        LOG(ERROR) << "TCPListener::HandleRead";

        if (errno == EMFILE)
        {
            ::close(idle_fd_);
            idle_fd_ = ::accept(accept_socket_.GetFd(), nullptr, nullptr);
            ::close(idle_fd_);
            idle_fd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}

void TCPListener::Listen()
{
    listenning_ = true;
    accept_socket_.Listen();
    accept_channel_.EnableReading();
}

