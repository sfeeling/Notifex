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

    void ShutdownWrite();
    static void ShutdownWrite(int sock_fd);

    void SetReuseAddr(bool on);

    static void SetNonBlockAndCloseOnExec(int sock_fd);

    static int CreateNonblockingOrDie(sa_family_t family);

    static ssize_t Read(int sockfd, void *buf, size_t count);
    static ssize_t Readv(int sockfd, const struct iovec *iov, int iovcnt);
    static ssize_t Write(int sockfd, const void *buf, size_t count);

    static int GetSocketError(int sock_fd);


private:
    const int sock_fd_;
};

}   // namespace notifex




#endif //NOTIFEX_SOCKET_H
