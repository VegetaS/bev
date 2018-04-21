#include "Channel.h"
#include "EventLoop.h"

#include <sstream>
#include <poll.h>

using namespace bev;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd) 
    : loop_(loop),
      fd_(fd),
      events_(0),
      revents_(0),
      index_(-1),
      logHup_(true),
      tied_(false),
      eventHandling_(false),
      addedToLoop_(false),
      eventIo_(new ev_io)
{
}

Channel::~Channel()
{
    assert(!eventHandling_);
    assert(!addedToLoop_);
    if (loop_->isInLoopThread())
    {
        assert(!loop_->hasChannel(this));
    }
}

void bev::Channel::update()
{
	addedToLoop_ = true;
}

void bev::Channel::onHandleEvent(struct ev_loop* loop, struct ev_io* io,
        int revents)
{
    Channel* channel = static_cast<Channel*>(io->data);
    channel->set_revents(revents);
    channel->handleEvent();
}

void bev::Channel::handleEvent()
{
	boost::shared_ptr<void> guard;
	if (tied_)
	{
		guard = tie_.lock();
		if (guard)
		{
			handleEventWithGuard();
		}
	}
	else
	{
		handleEventWithGuard();
	}
}

void bev::Channel::tie(const boost::shared_ptr<void>& obj)
{
	tied_ = true;
	tie_ = obj;
}

void bev::Channel::handleEventWithGuard()
{
    eventHandling_ = true;
    if ((revents_ & POLLHUP) && !(revents_ & POLLIN))
    {
        if (logHup_)
        {
            // TODO LOG
        }
        if (closeCallback_) closeCallback_();
    }

    if (revents_ & POLLNVAL)
    {
        // TODO
    }

    if (revents_ & (POLLERR | POLLNVAL))
    {
        if (errorCallback_) errorCallback_();
    }

    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP))
    {
        if (readCallback_) readCallback_();
    }

    if (revents_ & POLLOUT)
    {
        if (writeCallback_) writeCallback_();
    }

    eventHandling_ = false;
}


