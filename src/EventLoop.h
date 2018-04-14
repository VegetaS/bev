#ifndef BEV_SRC_EVENTLOOP_H 
#define BEV_SRC_EVENTLOOP_H


#include "Callbacks.h"
#include "CurrentThread.h"
#include "Mutex.h"

#include <vector>

#include <boost/noncopyable.hpp>
#include <ev++.h>

namespace bev {

    class EventLoop : boost::noncopyable
    {
        public:
            typedef boost::function<void()> Functor;

            EventLoop();
            ~EventLoop();

            void loop();
            void quit();

            void queueInLoop(const Functor& cb);
            void runInLoop(const Functor& cb);
            size_t queueSize() const;

            void assertInLoopThread()
            {
                if (!isInLoopThread())
                {
                    abortNotInLoopThread();
                }
            }

            bool isInLoopThread() const { return threadId_ == bev::tid(); }

            static EventLoop* getEventLoopOfCurrentThread();
        private:
            void abortNotInLoopThread();

            static void onWaked(struct ev_loop*, struct ev_async*, int);
            void wakeUp();
            void handlerWake();

            static void onDoPendingFunctors(struct ev_loop*, struct ev_check*, int);
            void doPendingFunctors();

        private:
            struct ev_loop* loop_;
            struct ev_async asyncWatcher_; 
            struct ev_check checkWatcher_;

            bool quit_;
            bool callingPendingFunctors_; /* atomic */

            mutable MutexLock mutex_;
            std::vector<Functor> pendingFunctors_; // @GuardedBy mutex_

            const pid_t threadId_;

    };
} // namespace bev

#endif
