//
// Created by sfeeling on 19-4-23.
//

#ifndef NOTIFEX_TCPCONNECTION_H
#define NOTIFEX_TCPCONNECTION_H

#include <memory>
#include <string>

#include "Buffer.h"
#include "Callbacks.h"

namespace notifex
{

class Channel;
class Socket;
class EventBase;


class TCPConnection : public std::enable_shared_from_this<TCPConnection>
{
public:
    TCPConnection(EventBase *event_base, const std::string &name, int sock_fd);
    ~TCPConnection();

    EventBase *GetBase() const { return event_base_; }
    const std::string &Name() const  { return name_; }
    bool Connected() const { return state_ == kConnected; }
    bool Disconnected() const { return state_ == kDisconnected; }

    void Send(std::string &&message);
    void Send(const void* message, int len);
    void Send(const StringPiece& message);
    void Send(Buffer *buf);
    void Shutdown();
    void ForceClose();

    void StartRead();
    void StopRead();
    bool IsReading() const { return reading_; }

    void SetConnectionCallback(const ConnectionCallback &cb)
    {
        connection_callback_ = cb;
    }

    void SetMessageCallback(const MessageCallback &cb)
    {
        message_callback_ = cb;
    }

    void SetWriteCompleteCallback(const WriteCompleteCallback &cb)
    {
        write_complete_callback_ = cb;
    }

    void  SetCloseCallback(const CloseCallback &cb)
    {
        close_callback_ = cb;
    }

    // 在TCP Server accepts了一个新连接后调用，调用一次
    void ConnectEstablished();
    // 被移除后调用，调用一次
    void ConnectDestroyed();

private:
    enum ConnState { kDisconnected, kConnecting, kConnected, kDisconnecting };

    void HandleRead();
    void HandleWrite();
    void HandleClose();
    void HandleError();

    void SendInBase(std::string &&message);
    void SendInBase(const StringPiece &message);
    void SendInBase(const void* message, size_t len);
    void ShutdownInBase();
    void ForceCloseInBase();
    void SetState(ConnState state) { state_ = state; }
    std::string StateToString() const;
    void StartReadInBase();
    void StopReadInBase();

    EventBase *event_base_;
    const std::string name_;
    ConnState state_;
    bool reading_;

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    // 考虑要不要保存端点信息Address

    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    WriteCompleteCallback write_complete_callback_;
    CloseCallback close_callback_;
    HighWaterMarkCallback high_water_mark_callback_;
    size_t high_water_mark_;

    Buffer input_buffer_;
    Buffer output_buffer_;
};

typedef std::shared_ptr<TCPConnection> TCPConnectionPtr;

}   // namespace notifex


#endif //NOTIFEX_TCPCONNECTION_H
