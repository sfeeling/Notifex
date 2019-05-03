//
// Created by sfeeling on 19-5-2.
//

#include "SockAddress.h"

#include <netdb.h>
#include <netinet/in.h>
#include <glog/logging.h>

#include "Endian.h"
#include "Socket.h"

// INADDR_ANY use (type)value casting.
#pragma GCC diagnostic ignored "-Wold-style-cast"
static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;
#pragma GCC diagnostic error "-Wold-style-cast"

using namespace notifex;

static_assert(offsetof(sockaddr_in, sin_family) == 0, "sin_family offset 0");
static_assert(offsetof(sockaddr_in, sin_port) == 2, "sin_port offset 2");

SockAddress::SockAddress(uint16_t port, bool loopback_only)
{
    static_assert(offsetof(SockAddress, addr_) == 0, "addr_ offset 0");
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    in_addr_t ip = loopback_only ? kInaddrLoopback : kInaddrAny;
    addr_.sin_addr.s_addr = HostToNetwork32(ip);
    addr_.sin_port = HostToNetwork16(port);
}

SockAddress::SockAddress(StringArg ip, uint16_t port)
{
    memset(&addr_, 0, sizeof(addr_));
    Socket::FromIpPort(ip.c_str(), port, &addr_);
}

std::string SockAddress::ToIp() const
{
    char buf[64] = "";
    Socket::ToIp(buf, sizeof(buf), GetSockAddr());
    return std::string(buf);
}

std::string SockAddress::ToIpPort() const
{
    char buf[64] = "";
    Socket::ToIpPort(buf, sizeof(buf), GetSockAddr());
    return std::string(buf);
}

uint16_t SockAddress::ToPort() const
{
    return NetworkToHost16(PortNetEndian());
}

uint32_t SockAddress::IpNetEndian() const
{
    assert(addr_.sin_family == AF_INET);
    return addr_.sin_addr.s_addr;
}

static __thread char t_resolve_buffer[64 * 1024];

bool SockAddress::Resolve(StringArg host_name, SockAddress *out)
{
    assert(out != nullptr);
    hostent hent;
    hostent *he = nullptr;
    int herrno = 0;
    memset(&hent, 0, sizeof(hent));

    int res = gethostbyname_r(host_name.c_str(), &hent, t_resolve_buffer, sizeof(t_resolve_buffer), &he, &herrno);
    if (res == 0 && he != nullptr)
    {
        assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
        out->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
        return true;
    }
    else
    {
        if (res)
            LOG(ERROR) << "SockAddress::Resolve";
        return false;
    }
}
