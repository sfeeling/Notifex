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

}

TCPConnection::TCPConnection(EventBase *event_base, const std::string &name, int sock_fd)
    :   event_base_(event_base/* TODO： 必须为非空 */),
        name_(name),
        state_(kConnecting),
        reading_(true),
        socket_(new Socket(sock_fd))
        // FIXME: Channel_(
{
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

void TCPConnection::Shutdown()
{

}

void TCPConnection::HandleRead()
{

}

void TCPConnection::HandleWrite()
{

}

void TCPConnection::HandleClose()
{

}

void TCPConnection::HandleError()
{

}

void TCPConnection::ForceClose()
{

}

void TCPConnection::StartRead()
{

}

void TCPConnection::StopRead()
{

}

void TCPConnection::ConnectEstablished()
{

}

void TCPConnection::ConnectDestroyed()
{

}

void TCPConnection::SendInBase(std::string &&message)
{

}

void TCPConnection::ShutdownInBase()
{

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

void TCPConnection::StopReadInBase()
{

}

void TCPConnection::StartReadInBase()
{

}

void TCPConnection::Send(std::string &&message)
{

}
