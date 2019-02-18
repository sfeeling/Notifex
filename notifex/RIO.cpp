//
// Created by sfeeling on 19-2-18.
//

#include "RIO.h"

#include <cerrno>
#include <unistd.h>

namespace notifex
{


ssize_t rio_readn(int fd, void *usr_buf, size_t n)
{
    size_t nleft = n;
    ssize_t nread;
    auto buf_ptr = static_cast<char *>(usr_buf);

    while (nleft > 0)
    {
        if ((nread = read(fd, buf_ptr, nleft)) < 0)
        {
            // 为了处理信号中断
            if (errno == EINTR)     // Interrupted by sig handler
                nread = 0;          // return and call read() again
            else
                return -1;      // errno是被read()设置的
        }
        else if (nread == 0)
            break;
        nleft -= nread;
        buf_ptr += nread;
    }

    // 返回传送的字节数
    return (n - nleft);
}

ssize_t rio_writen(int fd, void *usr_buf, size_t n)
{
    size_t nleft = n;
    ssize_t nwritten;
    auto buf_ptr = static_cast<char *>(usr_buf);
    while (nleft > 0)
    {
        // read()对应的是<0，读到0时表示结束或socket关闭
        if ((nwritten = write(fd, buf_ptr, nleft)) <= 0)
        {
            if (errno == EINTR)
                nwritten = 0;
            else
                return -1;
        }
        nleft -= nwritten;
        buf_ptr += nwritten;
    }

    return n;
}


}   // namespace notifex