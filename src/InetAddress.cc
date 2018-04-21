#include "InetAddress.h"
#include "Endian.h"
#include "SocketsOps.h"

#include <netdb.h>
#include <strings.h>  // bzero
#include <netinet/in.h>

#include <boost/static_assert.hpp>

#pragma GCC diagnostic ignored "-Wold-style-cast"
static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;
#pragma GCC diagnostic error "-Wold-style-cast"

using namespace bev;
using namespace bev::sockets;

BOOST_STATIC_ASSERT(sizeof(InetAddress) == sizeof(struct sockaddr_in6));
BOOST_STATIC_ASSERT(offsetof(sockaddr_in, sin_family) == 0);
BOOST_STATIC_ASSERT(offsetof(sockaddr_in6, sin6_family) == 0);
BOOST_STATIC_ASSERT(offsetof(sockaddr_in, sin_port) == 2);
BOOST_STATIC_ASSERT(offsetof(sockaddr_in6, sin6_port) == 2);

#if !(__GNUC_PREREQ (4,6))
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
InetAddress::InetAddress(uint16_t port, bool loopbackOnly, bool ipv6)
{
    BOOST_STATIC_ASSERT(offsetof(InetAddress, addr6_) == 0);
    BOOST_STATIC_ASSERT(offsetof(InetAddress, addr_) == 0);
    if (ipv6)
    {
        bzero(&addr6_, sizeof addr6_);
        addr6_.sin6_family = AF_INET6;
        in6_addr ip = loopbackOnly ? in6addr_loopback : in6addr_any;
        addr6_.sin6_addr = ip;
        addr6_.sin6_port = sockets::hostToNetwork16(port);
    }
    else
    {
        bzero(&addr_, sizeof addr_);
        addr_.sin_family = AF_INET;
        in_addr_t ip = loopbackOnly ? kInaddrLoopback : kInaddrAny;
        addr_.sin_addr.s_addr = sockets::hostToNetwork32(ip);
        addr_.sin_port = sockets::hostToNetwork16(port);
    }
}

InetAddress::InetAddress(StringArg ip, uint16_t port, bool ipv6)
{
    if (ipv6)
    {
        bzero(&addr6_, sizeof addr6_);
        sockets::fromIpPort(ip.c_str(), port, &addr6_);
    }
    else
    {
        bzero(&addr_, sizeof addr_);
        sockets::fromIpPort(ip.c_str(), port, &addr_);
    }
}

string InetAddress::toIpPort() const
{
      char buf[64] = "";
        sockets::toIpPort(buf, sizeof buf, getSockAddr());
          return buf;
}

string InetAddress::toIp() const
{
      char buf[64] = "";
        sockets::toIp(buf, sizeof buf, getSockAddr());
          return buf;
}

uint32_t InetAddress::ipNetEndian() const
{
      assert(family() == AF_INET);
        return addr_.sin_addr.s_addr;
}

uint16_t InetAddress::toPort() const
{
      return sockets::networkToHost16(portNetEndian());
}

static __thread char t_resolveBuffer[64 * 1024];

bool InetAddress::resolve(StringArg hostname, InetAddress* out)
{
    assert(out != NULL);
    struct hostent hent;
    struct hostent* he = NULL;
    int herrno = 0;
    bzero(&hent, sizeof(hent));

    int ret = gethostbyname_r(hostname.c_str(), &hent, t_resolveBuffer, sizeof t_resolveBuffer, &he, &herrno);
    if (ret == 0 && he != NULL)
    {
        assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
        out->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
        return true;
    }
    else
    {
        if (ret)
        {
            // TODO LOG
        }
        return false;
    }
}
