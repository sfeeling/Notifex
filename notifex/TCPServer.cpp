//
// Created by sfeeling on 19-4-25.
//

#include "TCPServer.h"

#include <cstdio>

#include "Callbacks.h"
#include "EventBase.h"
#include "TCPListener.h"
#include "Socket.h"

#include <glog/logging.h>

using namespace notifex;

void TCPServer::NewConnection(int sock_fd, const SockAddress &peer_addr)
{

    char buf[64];
    snprintf(buf, sizeof(buf), "-%s#%d", ip_port_.c_str(), next_conn_id);
    ++next_conn_id;
    std::string conn_name = name_ + buf;

    LOG(INFO) << "TcpServer::newConnection [" << name_
             << "] - new connection [" << conn_name
             << "] from " << peer_addr.ToIpPort();

    SockAddress local_addr(Socket::GetLocalAddr(sock_fd));


    TCPConnectionPtr conn(new TCPConnection(event_base_,
                                            conn_name,
                                            sock_fd,
                                            local_addr,
                                            peer_addr));
    connections_[conn_name] = conn;
    conn->SetConnectionCallback(connection_callback_);
    conn->SetMessageCallback(message_callback_);
    conn->SetWriteCompleteCallback(write_complete_callback_);
    conn->SetCloseCallback(
            std::bind(&TCPServer::RemoveConnection, this, _1));


    event_base_->GetThreadPool()->execute(
            std::bind(&TCPConnection::ConnectEstablished, conn));
    // FIXME: event_base_->RunInBase(std::bind(&TCPConnection::ConnectEstablished, conn));
}

void TCPServer::RemoveConnection(const TCPConnectionPtr &conn)
{
    event_base_->QueueInBase(std::bind(&TCPServer::RemoveConnectionInBase, this, conn));
}

void TCPServer::RemoveConnectionInBase(const TCPConnectionPtr &conn)
{

    LOG(INFO) << "TCPServer::RemoveConnectionInBase [" << name_
             << "] - connection " << conn->Name();

    size_t n = connections_.erase(conn->Name());
    LOG(INFO) << "删除后n为: " << n << "    " << conn->Name();
    (void) n;
    assert(n == 1);

    //conn->ConnectDestroyed();
    //event_base_->GetThreadPool()->execute(
           // std::bind(&TCPConnection::ConnectDestroyed, conn));
    event_base_->QueueInBase(std::bind(&TCPConnection::ConnectDestroyed, conn));
}

TCPServer::TCPServer(EventBase *event_base, const SockAddress &listen_addr, const std::string &name)
    :   event_base_(event_base), // FIXME: 需要检测非空
        ip_port_(listen_addr.ToIpPort()),
        name_(name),
        listener_(new TCPListener(event_base, listen_addr)),
        connection_callback_(DefaultConnectionCallback),
        message_callback_(DefaultMessageCallback),
        next_conn_id(1)
{
    listener_->SetNewConnectionCallback(
            std::bind(&TCPServer::NewConnection, this, _1, _2));
}


TCPServer::~TCPServer()
{
    LOG(INFO) << "TcpServer::~TcpServer [" << name_ << "] destructing";

    for (auto& item : connections_)
    {
        TCPConnectionPtr conn(item.second);
        item.second.reset();

        conn->GetBase()->GetThreadPool()->execute(
               std::bind(&TCPConnection::ConnectDestroyed, conn));
        // FIXME: conn->GetBase()->RunInBase(std::bind(&TCPConnection::ConnectDestroyed, conn));
    }
}

void TCPServer::Start()
{
    // FIXME: 参考muduo
    assert(!listener_->Listenning());

    event_base_->GetThreadPool()->execute(
            std::bind(&TCPListener::Listen, get_pointer(listener_)));
    // FIXME: event_base_->RunInBase(std::bind(&TCPListener::Listen, get_pointer(listener_)));
}


