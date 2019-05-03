#include <arpa/inet.h>
#include <memory.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <iostream>

#include "notifex.h"
#include "TCPServer.h"
#include "Socket.h"
#include "SockAddress.h"

#include <glog/logging.h>

using namespace std;
using namespace notifex;


void TimerEvent()
{
    cout << "Timer callback" << endl;
}



int main(int argc, char *argv[])
{

    google::InitGoogleLogging(argv[0]);

    // 输出到屏幕
    FLAGS_logtostderr = 1;


    EventBase event_base;
    SockAddress listen_addr(9898);
    TCPServer server(&event_base, listen_addr,
                     "EchoServer");
    server.SetConnectionCallback([](const TCPConnectionPtr &conn)
                                 {
                                     LOG(INFO) << "Echo服务器连接已"
                                                  << (conn->Connected() ? "开启" : "断开");
                                 });
    server.SetMessageCallback([](const TCPConnectionPtr &conn,
                                 Buffer *buf)
                              {
                                  string msg(buf->retrieveAllAsString());
                                  cout << msg;
                              });
    server.Start();
    event_base.NewDispatch();
}
