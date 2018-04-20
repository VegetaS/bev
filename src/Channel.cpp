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
      addedToLoop_(false)
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
}


