#ifndef BEV_NET_TCPCONNECTION_H
#define BEV_NET_TCPCONNECTION_H

#include "StringPiece.h"
#include "Types.h"
#include "Callbacks.h"
#include "Buffer.h"
#include "InetAddress.h"

#include <boost/any.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

struct tcp_info;

namespace bev
{
class Channel;
class EventLoop;
class Socket;

class TcpConnection : boost::noncopyable,
    public boost::enable_shared_from_this<TcpConnection>
{
    public:
        TcpConnection(EventLoop* loop, const string& name, int socketfd,
                const InetAddress& localAddr,
                const InetAddress& peerAddr);
        ~TcpConnection();
        EventLoop* getLoop() const { return loop_;}
    private:
          enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
          void handleRead();
          void handleWrite();
          void handleClose();
          void handleError();
          void sendInLoop(const StringPiece& message);
          void sendInLoop(const void* message, size_t len);
          void shutdownInLoop();
          void shutdownAndForceCloseInLoop(double seconds);
          void forceCloseInLoop();
          void setState(StateE s) { state_ = s; }
          const char* stateToString() const;
          void startReadInLoop();
          void stopReadInLoop();

    private:
        EventLoop* loop_;
        const string name_;
        StateE state_;
        bool reading_;
        boost::scoped_ptr<Socket> socket_;
        boost::scoped_ptr<Channel> channel_;
        const InetAddress localAddr_;
        const InetAddress peerAddr_;
        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;
        WriteCompleteCallback writeCompleteCallback_;
        HighWaterMarkCallback highWaterMarkCallback_;
        CloseCallback closeCallback_;
        size_t highWaterMark_;
        Buffer inputBuffer_;
        Buffer outputBuffer_;
        boost::any context_;
};

typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;

} // namespace bev

#endif 
