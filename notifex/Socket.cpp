//
// Created by sfeeling on 19-4-23.
//

#include "Socket.h"

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
