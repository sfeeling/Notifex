//
// Created by sfeeling on 19-4-23.
//

#include "Socket.h"
#include "Endian.h"
#include "Types.h"

#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>

#include <glog/logging.h>


using namespace notifex;

void Socket::Bind(const sockaddr *addr)
{
    int res = ::bind(sock_fd_, addr, sizeof(sockaddr_in));
    if (res < 0)
        LOG(FATAL) << "Socket::Bind";
}

void Socket::Listen()
{
    int res = ::listen(sock_fd_, SOMAXCONN);
    if (res < 0)
        LOG(FATAL) << "Socket::Listen";
}

int Socket::Accept(int sock_fd, sockaddr_in *addr)
{
    socklen_t addr_len = static_cast<socklen_t>(sizeof(*addr));
#if VALGRIND || defined (NO_ACCEPT4)
    int conn_fd = ::accept(sock_fd, SockaddrCast(addr), &addr_len);
    SetNonBlockAndCloseOnExec(conn_fd);
#else
    int conn_fd = ::accept4(sock_fd, SockaddrCast(addr),
                            &addr_len, SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif
    if (conn_fd < 0)
    {
        int saved_errno = errno;
        LOG(ERROR) << "Socket::Accept";
        switch (saved_errno)
        {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO: // ???
            case EPERM:
            case EMFILE: // per-process lmit of open file desctiptor ???
                // expected errors
                errno = saved_errno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                // unexpected errors
                LOG(FATAL) << "unexpected error of ::accept " << saved_errno;
                break;
            default:
                LOG(FATAL) << "unknown error of ::accept " << saved_errno;
                break;
        }
    }

    return conn_fd;
}

int Socket::Accept(sockaddr *cli_addr)
{
    socklen_t addr_len = sizeof(sockaddr_in);
    int conn_fd = ::accept(sock_fd_, cli_addr, &addr_len);
    SetNonBlockAndCloseOnExec(conn_fd);

    if (conn_fd < 0)
    {
        int saved_errno = errno;
        LOG(ERROR) << "Socket::Accept";
        switch (saved_errno)
        {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO: // ???
            case EPERM:
            case EMFILE: // per-process lmit of open file desctiptor ???
                // expected errors
                errno = saved_errno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                // unexpected errors
                LOG(FATAL) << "unexpected error of ::accept " << saved_errno;
                break;
            default:
                LOG(FATAL) << "unknown error of ::accept " << saved_errno;
                break;
        }
    }

    return conn_fd;
}

Socket::~Socket()
{
    if (close(sock_fd_) < 0)
        LOG(ERROR) << "Socket::~Socket";
}

void Socket::SetNonBlockAndCloseOnExec(int sock_fd)
{
    // 非阻塞
    int flags = ::fcntl(sock_fd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    int res = ::fcntl(sock_fd, F_SETFL, flags);
    if (res < 0)
        LOG(ERROR) << "Socket::SetNonBlockAndCloseOnExec";

    flags = ::fcntl(sock_fd, F_GETFD, 0);
    flags |= FD_CLOEXEC;
    res = ::fcntl(sock_fd, F_SETFD, flags);
    if (res < 0)
        LOG(ERROR) << "Socket::SetNonBlockAndCloseOnExec";
}

void Socket::SetReuseAddr(bool on)
{
    int opt_val = on ? 1 : 0;
    ::setsockopt(sock_fd_, SOL_SOCKET, SO_REUSEADDR,
                 &opt_val, static_cast<socklen_t>(sizeof(opt_val)));
}

int Socket::CreateNonblockingOrDie(sa_family_t family)
{
#if VALGRIND
    int sock_fd = ::socket(family, SOCK_STREAM, IPPROTO_TCP);
    if (sock_fd < 0)
    {
        LOG(FATAL) << "Socket::CreateNonblockingOrDie";
    }

    SetNonBlockAndCloseOnExec(sock_fd);
#else
    int sock_fd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sock_fd < 0)
    {
        LOG(FATAL) << "Socket::CreateNonblockingOrDie";
    }
#endif
    return sock_fd;
}

ssize_t Socket::Read(int sock_fd, void *buf, size_t count)
{
    return ::read(sock_fd, buf, count);
}

ssize_t Socket::Readv(int sock_fd, const struct iovec *iov, int iov_cnt)
{
    return ::readv(sock_fd, iov, iov_cnt);
}

ssize_t Socket::Write(int sock_fd, const void *buf, size_t count)
{
    return ::write(sock_fd, buf, count);
}

int Socket::GetSocketError(int sock_fd)
{
    int opt_val;
    socklen_t optlen = static_cast<socklen_t>(sizeof opt_val);

    if (::getsockopt(sock_fd, SOL_SOCKET, SO_ERROR, &opt_val, &optlen) < 0)
    {
        return errno;
    }
    else
    {
        return opt_val;
    }
}

void Socket::ShutdownWrite()
{
    ShutdownWrite(sock_fd_);
}

void Socket::ShutdownWrite(int sock_fd)
{
    if (::shutdown(sock_fd, SHUT_WR) < 0)
        LOG(ERROR) << "Socket::ShutdownWrite";
}

void Socket::ToIpPort(char *buf, size_t size, const sockaddr *addr)
{
    ToIp(buf, size, addr);
    size_t end = ::strlen(buf);
    const sockaddr_in *addr4 = SockaddrInCast(addr);
    uint16_t port = NetworkToHost16(addr4->sin_port);
    assert(size > end);
    snprintf(buf + end, size - end, ":%u", port);
}

void Socket::ToIp(char *buf, size_t size, const sockaddr *addr)
{
    assert(size >= INET_ADDRSTRLEN);
    const sockaddr_in *addr4 = SockaddrInCast(addr);
    ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
}

void Socket::FromIpPort(const char *ip, uint16_t port, sockaddr_in *addr)
{
    addr->sin_family = AF_INET;
    addr->sin_port = HostToNetwork16(port);
    if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
    {
        LOG(ERROR) << "Socket::FromIpPort";
    }
}

sockaddr * Socket::SockaddrCast(sockaddr_in *addr)
{
    return static_cast<sockaddr*>(implicit_cast<void *>(addr));
}

const sockaddr_in *Socket::SockaddrInCast(const sockaddr *addr)
{
    return static_cast<const sockaddr_in*>(implicit_cast<const void*>(addr));
}

bool Socket::IsSelfConnect(int sock_fd)
{
    return false;
}

sockaddr_in Socket::GetLocalAddr(int sock_fd)
{
    sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    socklen_t addr_len = static_cast<socklen_t>(sizeof(local_addr));
    if (::getsockname(sock_fd, SockaddrCast(&local_addr), &addr_len) < 0)
    {
        LOG(ERROR) << "Socket::GetLocalAddr";
    }
    return local_addr;
}

sockaddr_in Socket::GetPeerAddr(int sock_fd)
{
    sockaddr_in peer_addr;
    memset(&peer_addr, 0, sizeof(peer_addr));
    socklen_t addr_len = static_cast<socklen_t>(sizeof(peer_addr));
    if (::getpeername(sock_fd, SockaddrCast(&peer_addr), &addr_len) < 0)
    {
        LOG(ERROR) << "Socket::GetPeerAddr";
    }
    return peer_addr;
}

int Socket::Accept(SockAddress *peer_addr)
{
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    int conn_fd = Socket::Accept(sock_fd_, &addr);
    if (conn_fd >= 0)
        peer_addr->SetSockaddr(addr);
    return conn_fd;
}

void Socket::Bind(const SockAddress &addr)
{
    BindOrDie(sock_fd_, addr.GetSockAddr());
}

void Socket::BindOrDie(int sock_fd, const sockaddr *addr)
{
    int res = ::bind(sock_fd, addr, static_cast<socklen_t>(sizeof(sockaddr_in)));
    if (res < 0)
    {
        LOG(FATAL) << "Socket::BindOrDie";
    }
}

void Socket::ListenOrDie(int sock_fd)
{
    int res = ::listen(sock_fd, SOMAXCONN);
    if (res < 0)
    {
        LOG(FATAL) << "Socket::ListenOrDie";
    }
}



