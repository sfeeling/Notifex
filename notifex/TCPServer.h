//
// Created by sfeeling on 19-4-25.
//

#ifndef NOTIFEX_TCPSERVER_H
#define NOTIFEX_TCPSERVER_H


#include <sys/socket.h>

#include <atomic>
#include <map>

#include "Types.h"
#include "TCPConnection.h"


namespace notifex
{

class TCPListener;
class EventBase;

class TCPServer
{
public:
    enum Option
    {
        kNoReusePort,
        kReusePort
    };

    TCPServer(EventBase *event_base, const sockaddr *serv_addr, const std::string &name);
    ~TCPServer();

    const std::string &IpPort() const { return ip_port_; }
    const std::string &Name() const { return name_; }
    EventBase *GetBase() const { return event_base_; }

    void Start();

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

private:
    void NewConnection(int sock_fd);

    void RemoveConnection(const TCPConnectionPtr &conn);

    void RemoveConnectionInBase(const TCPConnectionPtr &conn);

    typedef std::map<std::string, TCPConnectionPtr> ConnectionMap;

    EventBase *event_base_;
    const std::string ip_port_;
    const std::string name_;
    std::unique_ptr<TCPListener> listener_;
    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    WriteCompleteCallback write_complete_callback_;
    std::atomic<int32_t> started_;

    int next_conn_id;
    ConnectionMap connections_;

};

}   // namespace notifex




#endif //NOTIFEX_TCPSERVER_H
