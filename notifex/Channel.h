//
// Created by sfeeling on 19-4-24.
//

#ifndef NOTIFEX_CHANNEL_H
#define NOTIFEX_CHANNEL_H

#include <functional>
#include <memory>
#include <string>
#include <glog/logging.h>


namespace notifex
{

class EventBase;

class Channel
{
public:
    typedef std::function<void()> EventCallback;

    Channel(EventBase *event_base, int fd);
    ~Channel();

    void HandleEvent();

    template <typename F>
    void SetReadCallback(F &&cb)
    {
        read_callback_ = std::forward<F>(cb);
    }

    template <typename F>
    void SetWriteCallback(F &&cb)
    {
        write_callback_ = std::forward<F>(cb);
    }

    template <typename F>
    void SetCloseCallback(F &&cb)
    {
        close_callback_ = std::forward<F>(cb);
    }

    template <typename F>
    void SetErrorCallback(F &&cb)
    {
        error_callback_ = std::forward<F>(cb);
    }

    int Fd() const { return fd_; }
    int Events() const { return events_; }
    void SetRevents(int revents) { revents_ = revents; }

    void Tie(const std::shared_ptr<void> &);

    bool IsNoneEvent() const { return  events_ == kNoneEvent; }

    void EnableReading() { events_ |= kReadEvent; Update(); }
    void DisableReading() { events_ &= ~kReadEvent; Update(); }
    void EnableWriting() { events_ |= kWriteEvent; Update(); }
    void DisableWriting() { events_ &= ~kWriteEvent; Update(); }
    void DisableAll() { events_ = kNoneEvent; Update(); }
    bool IsWriting() const { return events_ & kWriteEvent; }
    bool IsReading() const { return events_ & kReadEvent; }

    void AddToPool() { added_to_pool_ = true; }
    bool IsInPool() const { return added_to_pool_; }

    int Index() { return index_; }
    void SetIndex(int index) { index_ = index; }

    std::string EventsToString() const;
    std::string ReventsToString() const;

    EventBase *GetBase() { return event_base_; }
    void Remove();

private:
    static std::string EventsToString(int fd, int ev);

    void Update();
    void HandleEventSafely();


    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventBase *event_base_;
    const int fd_;
    int events_;
    int revents_;
    int index_; // 被Poller使用

    std::weak_ptr<void> tie_ptr_;
    bool tied_;
    bool event_handling_;
    bool added_to_base_;
    bool added_to_pool_;

    EventCallback read_callback_;
    EventCallback write_callback_;
    EventCallback close_callback_;
    EventCallback error_callback_;

};

}   // namespace notifex


#endif // NOTIFEX_CHANNEL_H
