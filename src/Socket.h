#ifndef BEV_SRC_SOCKET_H
#define BEV_SRC_SOCKET_H

#include <boost/noncopyable.hpp>

struct tcp_info;

namespace bev
{
    class InetAddress;

    class Socket : boost::noncopyable
    {
        public:
            explicit Socket(int sockfd)
                : sockfd_(sockfd)
            { }

            ~Socket();

            int fd() const { return sockfd_; }
            
            bool getTcpInfo(struct tcp_info*) const;
            bool getTcpInfoString(char* buf, int len) const;

            void bindAddress(const InetAddress& localaddr);
            void listen();

            int accept(InetAddress* peeraddr);

            void shutdownWrite();
 
            void setTcpNoDelay(bool on);

            void setReuseAddr(bool on);

            void setReusePort(bool on);

            void setKeepAlive(bool on);

        private:
            const int sockfd_;
    };

} // namespace bev

#endif  // BEV_SRC_SOCKET_H
