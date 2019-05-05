//
// Created by sfeeling on 19-4-22.
//

#ifndef NOTIFEX_TCPLISTENER_H
#define NOTIFEX_TCPLISTENER_H

#include <netinet/in.h>

#include <functional>

#include "Channel.h"
#include "Socket.h"
#include "SockAddress.h"

namespace notifex
{

class EventBase;

class Listener
{
public:
    typedef std::function<void (int sock_fd, const SockAddress&)> NewConnectionCallback;

    Listener(EventBase *event_base, const SockAddress &listen_addr);
    ~Listener();

    void SetNewConnectionCallback(const NewConnectionCallback &cb)
    { new_connection_callback_ = cb; }

    bool Listenning() const { return listenning_; }
    void Listen();

    inline const int GetListenFd()
    {
        return listen_fd_;
    }
    int Accept();

public:
    int listen_fd_;
    bool ev_in_;
    bool ev_out_;
    bool nonblock_;

    int res_;   // 回调函数的结果
    void *arg_;
    // 套接字
    sockaddr_in serv_addr_, cli_addr_;

public:
    void (*callback_)(int fd, int res, void *arg);   // 回调函数，事件触发时被调用

private:
    void HandleRead();

    EventBase *event_base_;
    Socket accept_socket_;
    Channel accept_channel_;
    NewConnectionCallback new_connection_callback_;
    bool listenning_;
    int idle_fd_;

};

}   // namespace notifex


#endif //NOTIFEX_TCPLISTENER_H
