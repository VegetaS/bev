#include "Mutex.h"
#include "EventLoop.h"
#include "Channel.h"

using namespace bev;

namespace
{
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


