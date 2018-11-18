//
// Created by sfeeling on 18-11-18.
//

#ifndef NOTIFEX_EVENT_H
#define NOTIFEX_EVENT_H

namespace notifex
{

class Event
{
public:
    explicit Event(int fd, void (*callback)(int, int, void *));
    ~Event() = default;
    Event(const Event &) = default;

    inline bool EventIn() const;
    inline bool EventOut() const;

public:
    int fd_;
    bool ev_in_;
    bool ev_out_;
    bool nonblock_;

    int res_;   // 回调函数的结果
    void *arg_;
    void (*callback_)(int fd, int res, void *arg);   // 回调函数，事件触发时被调用

};


inline bool Event::EventIn() const
{
    return ev_in_;
}

inline bool Event::EventOut() const
{
    return ev_out_;
}


}   // namespace notifex


#endif // NOTIFEX_EVENT_H
