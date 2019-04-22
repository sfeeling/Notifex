//
// Created by sfeeling on 19-4-22.
//

#include "TCPListener.h"

#include <cstring>

#include <glog/logging.h>

namespace notifex
{


TCPListener::TCPListener(uint16_t port, void (*callback)(int, int, void *))
    :   listen_fd_(socket(AF_INET, SOCK_STREAM, 0)),
        callback_(callback)
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



}   // namespace notifex