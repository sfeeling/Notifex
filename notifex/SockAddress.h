//
// Created by sfeeling on 19-5-2.
//

#ifndef NOTIFEX_SOCKADDRESS_H
#define NOTIFEX_SOCKADDRESS_H

#include <netinet/in.h>

#include "StringPiece.h"

namespace notifex
{

class SockAddress
{

public:
    explicit SockAddress(uint16_t port = 0, bool loopback_only = false);
    SockAddress(StringArg ip, uint16_t port);
    explicit SockAddress(const sockaddr_in &addr)
        :   addr_(addr)
    {}

    std::string ToIp() const;
    std::string ToIpPort() const;
    uint16_t ToPort() const;

    const sockaddr *GetSockAddr() const
    {
        return static_cast<const sockaddr *>(implicit_cast<const void*>(&addr_));
    }
    void SetSockaddr(const sockaddr_in &addr)
    {
        addr_ = addr;
    }

    uint32_t IpNetEndian() const;
    uint16_t PortNetEndian() const { return addr_.sin_port; }

    static bool Resolve(StringArg host_name, SockAddress *result);

private:
    sockaddr_in addr_;

};



}   // namespace notifex




#endif //NOTIFEX_SOCKADDRESS_H
