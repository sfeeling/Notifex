#include <arpa/inet.h>
#include <memory.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include "notifex.h"

using namespace std;

void ReadEvent(int fd, int res, void *arg)
{
    const int READ_SIZE = 32;
    char *buf = new char[READ_SIZE];
    int n;
    bool done = false;
    while (!done)
    {
        n = read(fd, buf, READ_SIZE);
        if (n > 0)
        {
            cout << "stdin event" << endl;
            write(STDOUT_FILENO, buf, n);
        }
        else if (n == 0)
        {
            cout << "fd closed: " << fd << endl;
            done = true;
        }
        else
        {
            if (errno == EWOULDBLOCK)
                done = true;
        }
    }

    cout << "read with empty buf: " << read(fd, buf, 0) << endl;

    delete[] buf;
}

void TimerEvent()
{
    cout << "Timer callback" << endl;
}

int main()
{
	notifex::EventBase event_base;
	event_base.Debug();
	notifex::Event ev_in(0, ReadEvent);
	notifex::Timer ev_timer(5, 0, TimerEvent);

	/*
	sockaddr_in serv_addr, cli_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(7000);
	inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

	int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	bind(listen_fd, (sockaddr*)&serv_addr, sizeof(serv_addr));
	listen(listen_fd, 5);

	socklen_t cli_len = sizeof(cli_addr);
	int fd = accept(listen_fd, (sockaddr*)&cli_addr, &cli_len);
	notifex::Event ev_sock(fd, ReadEvent);
	 */

	event_base.AddEvent(ev_in);
	event_base.AddTimer(ev_timer);
	//event_base.AddEvent(ev_sock);
	event_base.Dispatch();
}
