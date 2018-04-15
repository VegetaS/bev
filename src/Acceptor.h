#ifndef BEV_SRC_ACCEPTOR_H
#define BEV_SRC_ACCEPTOR_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include "Channel.h"
#include "Socket.h"

extern "C"
{
#include <ev.h>
}

namespace bev {
    class EventLoop;
    class InetAddress;

    class Acceptor : boost:noncopyable
    {
        public:
            typedef boost::function<void (int sockfd, const InetAddress&)> NewConnectionCallback;

            Acceptor(EventLoop* loop, InetAddress& listenAddr, bool reusePort);
            ~Acceptor();

            void setNewConnectionCallback(const NewConnectionCallback& cb) { newConnectionCallback_ = cb;}
            bool listening() const { return listening_; }
            void listen();

        private:
            void handleRead();

        private:
            EventLoop* loop_;
            Socket acceptSocket_;
            Channel acceptChannel_;
            NewConnectionCallback newConnectionCallback_;
            bool listenning_;
            int idleFd_;
    };

} // namespace bev

#endif

