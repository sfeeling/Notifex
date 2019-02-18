//
// Created by sfeeling on 19-2-18.
//
// RIO means Robust I/O，健壮的I/O
//

#ifndef NOTIFEX_RIO_H
#define NOTIFEX_RIO_H

#include <cstdio>

namespace notifex
{

// rio的无缓冲的输入输出函数
ssize_t rio_readn(int fd, void *usr_buf, size_t n);
ssize_t rio_writen(int fd, void *usr_buf, size_t n);
// 若成功则返回传送点字节数，若EOF则为0（只对rio_readn而言），若出错返回-1



}   // namespace notifex



#endif //NOTIFEX_RIO_H
