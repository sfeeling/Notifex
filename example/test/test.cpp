#include <arpa/inet.h>
#include <memory.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include "notifex.h"

#include <functional>
#include <memory>

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
	ev_timer.SetRepeated();



	event_base.AddEvent(ev_in);
	event_base.AddTimer(ev_timer);
	//event_base.AddEvent(ev_sock);
	event_base.Dispatch();

    // TODO 未定义的引用
    //char buf[100];
    //notifex::rio_readn(STDIN_FILENO, buf, 10);
    //notifex::rio_writen(STDOUT_FILENO, buf, 20);

}
