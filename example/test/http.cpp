#include <arpa/inet.h>
#include <fcntl.h>
#include <memory.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <iostream>

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
    const int BUFF_SIZE = 512;
    char buff[BUFF_SIZE];
    int n;

    const char text[] =
            "HTTP/1.1 200 OK\r\n"
            "Server: notifex/1.2.1\r\n"
            "Content-Type: text/html;Charset=utf-8\r\n"
            "Connection: keep-alive\r\n"
            "\r\n";
    server.SetMessageCallback([&](const TCPConnectionPtr &conn,
                                 Buffer *buf)
                              {
                                  string msg(buf->retrieveAllAsString());
                                  cout << msg;

                                  conn->Send(text, sizeof(text));
                                  int file_fd = open("poem.html", O_RDONLY);
                                  n = read(file_fd, buff, BUFF_SIZE);
                                  conn->Send(buff, n);
                                  conn->ForceClose();
                              });
    server.Start();
    event_base.NewDispatch();
}
