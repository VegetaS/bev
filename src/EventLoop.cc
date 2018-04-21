#include "Mutex.h"
#include "EventLoop.h"
#include "Channel.h"

#include <sys/epoll.h>

using namespace bev;

namespace
{
	const int kNew = -1;
	const int kAdded = 1;
	const int kDeleted = 2;

	__thread EventLoop* t_loopInThisThread = 0;
#pragma GCC diagnostic ignored "-Wold-style-cast"
    class IgnoreSigPipe
    {
        public:
            IgnoreSigPipe()
            {
                ::signal(SIGPIPE, SIG_IGN);
                // LOG_TRACE << "Ignore SIGPIPE";
            }
    };
#pragma GCC diagnostic error "-Wold-style-cast"

    IgnoreSigPipe initObj;
}

EventLoop::EventLoop()
    : quit_(false),
      callingPendingFunctors_(false),
      threadId_(bev::tid())
{
    loop_ = static_cast<struct ev_loop*>(ev_loop_new(EVFLAG_AUTO));
    ev_set_userdata(loop_, this);

    ev_async_init(&asyncWatcher_, &EventLoop::onWaked);
    ev_check_init(&checkWatcher_, &EventLoop::onDoPendingFunctors);
    ev_set_priority(&checkWatcher_, EV_MAXPRI);

    ev_async_start(loop_, &asyncWatcher_);
    ev_check_start(loop_, &checkWatcher_);
}

EventLoop::~EventLoop()
{
    ev_async_stop(loop_, &asyncWatcher_);
    ev_check_stop(loop_, &checkWatcher_);

    ev_loop_destroy(loop_);
}

void EventLoop::loop()
{
    quit_ = false;
    ev_run(loop_, 0);
}

void EventLoop::quit()
{
    quit_ = true;
    ev_break(loop_, EVBREAK_ALL);    
    wakeUp();
}

void bev::EventLoop::updateChannel(Channel *channel)
{
	assert(channel->ownerLoop() == this);
	assertInLoopThread();

	const int index = channel->index();
	if (index == kNew || index == kDeleted)
	{
		int fd = channel->fd();
		if (index == kNew)
		{
			assert(channels_.find(fd) == channels_.end());
			channels_[fd] = channel;
		}
		else
		{
			assert(channels_.find(fd) != channels_.end());
			assert(channels_[fd] == channel);
		}

		channel->set_index(kAdded);
		update(EPOLL_CTL_ADD, channel);
	}
	else
	{
		int fd = channel->fd();
		(void)fd;
		assert(channels_.find(fd) != channels_.end());
		assert(channels_[fd] == channel);
		assert(index == kAdded);
		if (channel->isNoneEvent())
		{
			update(EPOLL_CTL_DEL, channel);
			channel->set_index(kDeleted);
		}
		else
		{
			update(EPOLL_CTL_MOD, channel);
		}
	}
}

EventLoop * bev::EventLoop::getEventLoopOfCurrentThread()
{
	return t_loopInThisThread;
}

void bev::EventLoop::abortNotInLoopThread()
{
	// TODO LOG_FATAL
}

void EventLoop::onWaked(struct ev_loop* loop, struct ev_async* w, int event)
{
    EventLoop* evloop = static_cast<EventLoop*>(ev_userdata(loop));
    evloop->handlerWake();
}

void EventLoop::wakeUp()
{
    ev_async_send(loop_, &asyncWatcher_);
}

void EventLoop::handlerWake()
{
    if (quit_)
    {
        ev_break(loop_, EVBREAK_ALL);
        return;
    }
}

void bev::EventLoop::update(int operation, Channel * channel)
{
    struct ev_io *ev = channel->eventIo();
    if (operation == EPOLL_CTL_DEL)
    {
        ev_io_stop(loop_, ev);
        return;
    }

    int fd = channel->fd();
    int events = channel->events();
    ev->data = channel;
    ev_io_init(ev, &Channel::onHandleEvent, fd, events);
    ev_io_start(loop_, ev);
}

void EventLoop::queueInLoop(const Functor& cb)
{
    {
        MutexLockGuard lock(mutex_);
        pendingFunctors_.push_back(cb);
    }

    if (isInLoopThread() || callingPendingFunctors_)
    {
        wakeUp();
    }
}

void EventLoop::runInLoop(const Functor& cb)
{
    if (isInLoopThread())
    {
        cb();
    }
    else
    {
        queueInLoop(cb);
    }
}

size_t EventLoop::queueSize() const
{
    MutexLockGuard lock(mutex_);
    return pendingFunctors_.size();
}


bool EventLoop::hasChannel(Channel* channel)
{
    // TODO
}

void EventLoop::onDoPendingFunctors(struct ev_loop* loop, struct ev_check* w, int events)
{
    EventLoop* evloop = static_cast<EventLoop*>(ev_userdata(loop));
    evloop->doPendingFunctors();
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        MutexLockGuard lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (size_t i = 0; i < functors.size(); ++i)
    {
        functors[i]();
    }

    callingPendingFunctors_ = false;
}


