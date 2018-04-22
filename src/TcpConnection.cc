#include "TcpConnection.h"
#include "WeakCallback.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"
#include "SocketsOps.h"

#include <boost/bind>

#include <errno.h>

using namespace bev;

void bev::defaultConnectionCallback(const TcpConnectionPtr& conn)
{
    // TODO
}

void bev::defaultMessageCallback(const TcpConnectionPtr& conn, Buffer* buf)
{
    buf->retrieveAll();
}

TcpConnection::TcpConnection(EventLoop* loop
        const string& name,
        int socketfd,
        const InetAddress& localAddr,
        const InetAddress& peerAddr)
    : loop_(loop),
      name_(name),
      state_(kConnecting),
      reading_(true),
      socket_(new Socket(sockfd)),
      channel_(new Channel(loop, sockfd)),
      localAddr_(localAddr),
      peerAddr_(peerAddr),
      highWaterMark_(64*1024*1024)
{
    channel_->setReadCallback(
            boost::bind(&TcpConnection::handleRead, this));
    channel_->setWriteCallback(
            boost::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(
            boost::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(
            boost::bind(&TcpConnection::handleError, this));
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
    // TODO LOG
}


bool TcpConnection::getTcpInfo(struct tcp_info* tcpinfo) const 
{
    return socket_->getTcpInfo(tcpinfo);
}

string TcpConnection::getTcpInfoString() const 
{
    char buf[1024];
    buf[0] = '\0';
    socket_->getTcpInfoString(buf, sizeof buf);
    return buf;
}

void TcpConnection::Send(const void *data, int len)
{
    send(StringPiece(static_cast<const char*>(data), len));
}

void TcpConnection::send(const StringPiece& message)
{
    if (state_ == kConnected)
    {
        if (loop_->isInLoopThread())
        {
            sendInLoop(message);
        }
        else
        {
            loop_->runInLoop(
                    boost::bind(&TcpConnection::sendInLoop, 
                        this, 
                        message.as_string()));
        }
    }
}

void TcpConnection::send(Buffer* buf)
{
    if (state_ == kConnected)
    {
        if (loop_->isInLoopThread())
        {
            sendInLoop(buf->peek(), buf->readableBytes());
            buf->retrieveAll();
        }
        else
        {
            loop_->runInLoop(boost::bind(&TcpConnection::sendInLoop,
                        this,
                        buf->retrieveAllAsString()));
        }
    }
}

void TcpConnection::sendInLoop(const StringPiece& message)
{
    sendInLoop(message.data(), message.size());
}

void TcpConnection::sendInLoop(const void* data, size_t len)
{
}





















