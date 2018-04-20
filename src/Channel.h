#ifndef BEV_SRC_CHANNEL_H
#define BEV_SRC_CHANNEL_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>


namespace bev
{
    class EventLoop;

    class Channel : boost::noncopyable
    {
        public:
            typedef boost::function<void()> EventCallback;
            typedef boost::function<void()> ReadEventCallback;
            
            Channel(EventLoop* loop, int fd);
            ~Channel();

            void handleEvent();
            void setReadCallback(const ReadEventCallback& cb)
            { readCallback_ = cb; }
            void setWriteCallback(const EventCallback& cb)
            { writeCallback_ = cb; }
            void setCloseCallback(const EventCallback& cb)
            { closeCallback_ = cb; }
            void setErrorCallback(const EventCallback& cb)
            { errorCallback_ = cb; }

            void tie(const boost::shared_ptr<void>&);
            int fd() const { return fd_; }
            int events() const { return events_; }
            void set_revents(int revt) { revents_ = revt; } // used by pollers
            bool isNoneEvent() const { return events_ == kNoneEvent; }

            void enableReading() { events_ |= kReadEvent; update(); }
            void disableReading() { events_ &= ~kReadEvent; update(); }
            void enableWriting() { events_ |= kWriteEvent; update(); }
            void disableWriting() { events_ &= ~kWriteEvent; update(); }
            void disableAll() { events_ = kNoneEvent; update(); }
            bool isWriting() const { return events_ & kWriteEvent; }
            bool isReading() const { return events_ & kReadEvent; }

            int index() { return index_; }
            void set_index(int idx) { index_ = idx; }
            void doNotLogHup() { logHup_ = false; }

            EventLoop* ownerLoop() { return loop_; }
            void remove();

        private:
            static string eventsToString(int fd, int ev);

            void update();
            void handleEventWithGuard();

            static const int kNoneEvent;
            static const int kReadEvent;
            static const int kWriteEvent;

            EventLoop* loop_;
            const int  fd_;
            int        events_;
            int        revents_; // it's the received event types of epoll or poll
            int        index_; // used by Poller.
            bool       logHup_;

            boost::weak_ptr<void> tie_;
            bool tied_;
            bool eventHandling_;
            bool addedToLoop_;
            ReadEventCallback readCallback_;
            EventCallback writeCallback_;
            EventCallback closeCallback_;
            EventCallback errorCallback_;
    };

} // namespace bev
#endif
