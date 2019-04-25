//
// Created by sfeeling on 19-4-23.
//

#ifndef NOTIFEX_CALLBACKS_H
#define NOTIFEX_CALLBACKS_H

#include <cassert>
#include <functional>
#include <memory>

#include "Types.h"

// 在读的部分消去了时间戳参数
namespace notifex
{

// 用于后续的参数绑定
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

template<typename T>
inline T *get_pointer(const std::shared_ptr<T>& ptr)
{
    return ptr.get();
}

template<typename T>
inline T *get_pointer(const std::unique_ptr<T>& ptr)
{
    return ptr.get();
}

template<typename To, typename From>
inline ::std::shared_ptr<To> down_pointer_cast(const ::std::shared_ptr<From>& f) {
    if (false)
    {
        implicit_cast<From*, To*>(0);
    }

#ifndef NDEBUG
    assert(f == NULL || dynamic_cast<To*>(get_pointer(f)) != NULL);
#endif
    return ::std::static_pointer_cast<To>(f);
}

class Buffer;
class TCPConnection;

typedef std::shared_ptr<TCPConnection> TCPConnectionPtr;
typedef std::function<void (const TCPConnectionPtr&)> ConnectionCallback;
typedef std::function<void (const TCPConnectionPtr&)> CloseCallback;
typedef std::function<void (const TCPConnectionPtr&)> WriteCompleteCallback;
typedef std::function<void (const TCPConnectionPtr&, size_t)> HighWaterMarkCallback;

typedef std::function<void (const TCPConnectionPtr&,
                            Buffer*)> MessageCallback;

void DefaultConnectionCallback(const TCPConnectionPtr& conn);
void DefaultMessageCallback(const TCPConnectionPtr& conn, Buffer* buffer);

}   // namespace notifex

#endif //NOTIFEX_CALLBACKS_H
