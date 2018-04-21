#ifndef BEV_SRC_EVENTLOOP_H 
#define BEV_SRC_EVENTLOOP_H


#include "Callbacks.h"
#include "CurrentThread.h"
#include "Mutex.h"

#include <vector>
#include <map>

#include <boost/noncopyable.hpp>
#include <ev++.h>

namespace bev {

    class Channel;

    class EventLoop : boost::noncopyable
    {
        public:
            typedef boost::function<void()> Functor;
			typedef std::map<int, Channel*> ChannelMap;

            EventLoop();
            ~EventLoop();

            void loop();
            void quit();

            void queueInLoop(const Functor& cb);
            void runInLoop(const Functor& cb);
            size_t queueSize() const;

            bool hasChannel(Channel* channel);

            void assertInLoopThread()
            {
                if (!isInLoopThread())
                {
                    abortNotInLoopThread();
                }
            }

            bool isInLoopThread() const { return threadId_ == bev::tid(); }
			void updateChannel(Channel*);
            static EventLoop* getEventLoopOfCurrentThread();

        private:
            void abortNotInLoopThread();

            static void onWaked(struct ev_loop*, struct ev_async*, int);
            void wakeUp();
            void handlerWake();
			void update(int operation, Channel* channel);

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

			ChannelMap channels_;

    };
} // namespace bev

#endif
