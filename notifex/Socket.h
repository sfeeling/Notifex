//
// Created by sfeeling on 19-4-23.
//

#ifndef NOTIFEX_SOCKET_H
#define NOTIFEX_SOCKET_H

#include <arpa/inet.h>
#include <sys/socket.h>
#include "SockAddress.h"

namespace notifex
{

class Socket
{
public:
    explicit Socket(int sock_fd)
        :   sock_fd_(sock_fd)
    { }

    ~Socket();

    int Fd() const { return sock_fd_; }

    void Bind(const sockaddr *addr);
    void Bind(const SockAddress &addr);
    void Listen();
    int Accept(sockaddr *cli_addr);
    int Accept(SockAddress *peer_addr);

    void ShutdownWrite();
    static void ShutdownWrite(int sock_fd);

    void SetReuseAddr(bool on);

    // static作用于所有Socket的对象
    static void SetNonBlockAndCloseOnExec(int sock_fd);

    static int CreateNonblockingOrDie(sa_family_t family);

    static void BindOrDie(int sock_fd, const sockaddr *addr);
    static void ListenOrDie(int sock_fd);
    static int Accept(int sock_fd, sockaddr_in *addr);

    static ssize_t Read(int sockfd, void *buf, size_t count);
    static ssize_t Readv(int sockfd, const struct iovec *iov, int iovcnt);
    static ssize_t Write(int sockfd, const void *buf, size_t count);

    static int GetSocketError(int sock_fd);

    static void ToIpPort(char *buf, size_t size, const sockaddr *addr);
    static void ToIp(char *buf, size_t size, const sockaddr *addr);

    static void FromIpPort(const char* ip, uint16_t port, sockaddr_in* addr);

    static sockaddr * SockaddrCast(sockaddr_in *addr);
    static const sockaddr_in *SockaddrInCast(const sockaddr *addr);

    static sockaddr_in GetLocalAddr(int sock_fd);
    static sockaddr_in GetPeerAddr(int sock_fd);
    static bool IsSelfConnect(int sock_fd);
private:
    const int sock_fd_;
};

}   // namespace notifex




#endif //NOTIFEX_SOCKET_H
