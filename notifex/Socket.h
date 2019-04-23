//
// Created by sfeeling on 19-4-23.
//

#ifndef NOTIFEX_SOCKET_H
#define NOTIFEX_SOCKET_H

#include <sys/socket.h>

namespace notifex
{

class Socket
{
public:
    explicit Socket(int sock_fd)
        :   sock_fd_(sock_fd)
    { }

    ~Socket();

    int GetFd() const { return sock_fd_; }

    void Bind(const sockaddr *addr);
    void Listen();
    int Accept(sockaddr *cli_addr);

    void SetReuseAddr(bool on);

    void SetNonBlock(int sock_fd);

private:
    const int sock_fd_;
};

}   // namespace notifex




#endif //NOTIFEX_SOCKET_H
