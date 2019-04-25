#include <arpa/inet.h>
#include <memory.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <iostream>

#include "notifex.h"
#include "TCPServer.h"
#include "Socket.h"

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

    sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = ::htonl(INADDR_ANY);
    serv_addr.sin_port = ::htons(9898);

    EventBase event_base;
    TCPServer server(&event_base, (sockaddr *)&serv_addr,
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
