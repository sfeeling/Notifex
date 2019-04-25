//
// Created by sfeeling on 19-4-23.
//

#include "TCPConnection.h"

#include <cerrno>

#include "EventBase.h"
#include "Socket.h"

#include <glog/logging.h>



using namespace notifex;

void notifex::DefaultConnectionCallback(const TCPConnectionPtr& conn)
{
    LOG(INFO) << "New connection is " << (conn->Connected() ? "UP" : "DOWN");
}

void notifex::DefaultMessageCallback(const TCPConnectionPtr& conn, Buffer* buffer)
{
    // FIXME: 这里需要把数据从buf取出来
}

TCPConnection::TCPConnection(EventBase *event_base, const std::string &name, int sock_fd)
    :   event_base_(event_base/* TODO： 必须为非空 */),
        name_(name),
        state_(kConnecting),
        reading_(true),
        socket_(new Socket(sock_fd)),
        channel_(new Channel(event_base, sock_fd))
        // FIXME: Channel_(
{
    channel_->SetReadCallback(
            std::bind(&TCPConnection::HandleRead, this));
    channel_->SetWriteCallback(
            std::bind(&TCPConnection::HandleWrite, this));
    channel_->SetCloseCallback(
            std::bind(&TCPConnection::HandleClose, this));
    channel_->SetErrorCallback(
            std::bind(&TCPConnection::HandleError, this));
    channel_->AddToPool();

    LOG(INFO) << "TCPConnection::ctor[" << name_ << "] at " << this
              << " fd=" << sock_fd;
    // TODO: KEEPALIVE
}

TCPConnection::~TCPConnection()
{
    LOG(INFO) << "TCPConnection::dtor[" << name_ << "] at " << this
              << " fd=" << "" // FIXME: channel->fd()
              << " state=" << StateToString();
    assert(state_ == kDisconnected);
}

void TCPConnection::HandleRead()
{
    int saved_errno = 0;
    ssize_t n = input_buffer_.ReadFd(channel_->Fd(), &saved_errno);
    if (n > 0)
    {
        message_callback_(shared_from_this(), &input_buffer_);
    }
    else if (n == 0)
    {
        HandleClose();
    }
    else
    {
        errno = saved_errno;
        LOG(ERROR) << "TCPConnection::HandleRead";
        HandleError();
    }
}

void TCPConnection::HandleWrite()
{
    if (channel_->IsWriting())
    {
        ssize_t n = Socket::Write(channel_->Fd(),
                                   output_buffer_.peek(),
                                   output_buffer_.ReadableBytes());
        if (n > 0)
        {
            output_buffer_.retrieve(n);
            if (output_buffer_.ReadableBytes() == 0)
            {
                channel_->DisableWriting();
                if (write_complete_callback_)
                {
                    event_base_->GetThreadPool()->execute(
                            std::bind(write_complete_callback_, shared_from_this()));
                    // FIXME: event_base_->QueueInBase(std::bind(write_complete_callback_, shared_from_this()));
                }
                if (state_ == kDisconnecting)
                {
                    ShutdownInBase();
                }
            }
        }
        else
        {
            LOG(ERROR) << "TCPConnection::HandleWrite";
            // if (state_ == kDisconnecting)
            // {
            //   shutdownInLoop();
            // }
        }
    }
    else
    {
        LOG(INFO) << "Connection fd = " << channel_->Fd()
                  << " is down, no more writing";
    }
}

void TCPConnection::HandleClose()
{
    LOG(INFO) << "fd = " << channel_->Fd()
              << " state = " << StateToString();
    assert(state_ == kConnected || state_ == kDisconnecting);
    SetState(kDisconnected);
    channel_->DisableAll();

    TCPConnectionPtr guard_this(shared_from_this());
    connection_callback_(guard_this);
    close_callback_(guard_this);
}

void TCPConnection::HandleError()
{
    int err = Socket::GetSocketError(channel_->Fd());
    LOG(ERROR) << "TcpConnection::handleError [" << name_
              << "] - SO_ERROR = " << err;
}

void TCPConnection::ForceClose()
{

}

void TCPConnection::StartRead()
{
    event_base_->GetThreadPool()->execute(
            std::bind(&TCPConnection::StartReadInBase, this));
    // FIXME: event_base_->RunInBase(std::bind(&TCPConnection::StartReadInBase, this));
}

void TCPConnection::StartReadInBase()
{
    if (!reading_ || !channel_->IsReading())
    {
        channel_->EnableReading();
        reading_ = true;
    }
}

void TCPConnection::StopRead()
{
    event_base_->GetThreadPool()->execute(
            std::bind(&TCPConnection::StopReadInBase, this));
    // FIXME: event_base_->RunInBase(std::bind(&TCPConnection::StopReadInBase, this));
}

void TCPConnection::StopReadInBase()
{
    if (reading_ || channel_->IsReading())
    {
        channel_->DisableReading();
        reading_ = false;
    }
}

void TCPConnection::ConnectEstablished()
{
    assert(state_ == kConnecting);
    SetState(kConnected);
    channel_->Tie(shared_from_this());
    channel_->EnableReading();

    // why???
    connection_callback_(shared_from_this());
}

void TCPConnection::ConnectDestroyed()
{
    if (state_ == kConnected)
    {
        SetState(kDisconnected);
        channel_->DisableAll();

        connection_callback_(shared_from_this());
    }
    channel_->Remove();
}

void TCPConnection::SendInBase(std::string &&message)
{

}

void TCPConnection::Shutdown()
{
    if (state_ == kConnected)
    {
        SetState(kDisconnecting);
        event_base_->GetThreadPool()->execute(
                std::bind(&TCPConnection::ShutdownInBase, this));
        // FIXME: event_base_->RunInBase(std::bind(&TCPConnection::ShutdownInBase, this));
    }
}

void TCPConnection::ShutdownInBase()
{
    if (!channel_->IsWriting())
        socket_->ShutdownWrite();
}

void TCPConnection::ForceCloseInBase()
{

}

std::string TCPConnection::StateToString() const
{
    switch (state_)
    {
        case kConnected:
            return "kConnected";
        case kConnecting:
            return "kConnecting";
        case kDisconnected:
            return "kDisconnected";
        case kDisconnecting:
            return "kDisconnecting";
    }
}


void TCPConnection::Send(std::string &&message)
{

}

void TCPConnection::Send(const void *message, int len)
{
    Send(StringPiece(static_cast<const char*>(message), len));
}

void TCPConnection::Send(const StringPiece &message)
{
    if (state_ == kConnected)
    {
        SendInBase(message);

    }
}

void TCPConnection::Send(notifex::Buffer *buf)
{
    if (state_ == kConnected)
    {
        SendInBase(buf->peek(), buf->ReadableBytes());
        buf->retrieveAll();
    }
}

void TCPConnection::SendInBase(const StringPiece &message)
{
    SendInBase(message.data(), message.size());
}

void TCPConnection::SendInBase(const void *message, size_t len)
{
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;
    if (state_ == kDisconnected)
    {
        LOG(WARNING) << "disconnected, give up writing";
        return;
    }
    // if no thing in output queue, try writing directly
    if (!channel_->IsWriting() && output_buffer_.ReadableBytes() == 0)
    {
        nwrote = Socket::Write(channel_->Fd(), message, len);
        if (nwrote >= 0)
        {
            remaining = len - nwrote;
            if (remaining == 0 && write_complete_callback_)
            {
                event_base_->GetThreadPool()->execute(
                        std::bind(write_complete_callback_, shared_from_this()));
                // FIXME:: event_base_->QueueInBase(std::bind(write_complete_callback_, shared_from_this()));
            }
        }
        else // nwrote < 0
        {
            nwrote = 0;
            if (errno != EWOULDBLOCK)
            {
                LOG(ERROR) << "TcpConnection::sendInLoop";
                if (errno == EPIPE || errno == ECONNRESET) // FIXME: any others?
                {
                    faultError = true;
                }
            }
        }
    }

    assert(remaining <= len);
    if (!faultError && remaining > 0)
    {
        size_t oldLen = output_buffer_.ReadableBytes();
        if (oldLen + remaining >= high_water_mark_
            && oldLen < high_water_mark_
            && high_water_mark_callback_)
        {
            event_base_->GetThreadPool()->execute(
                    std::bind(high_water_mark_callback_, shared_from_this(), oldLen + remaining));
            // FIXME: event_base_->QueueInBase(std::bind(high_water_mark_callback_, shared_from_this(), oldLen + remaining));
        }
        output_buffer_.append(static_cast<const char*>(message)+nwrote, remaining);
        if (!channel_->IsWriting())
        {
            channel_->EnableWriting();
        }
    }
}


