cmake_minimum_required(VERSION 3.10)
project(Notifex)

set(CMAKE_CXX_STANDARD 14)

add_library(notifex SHARED notifex/notifex.h notifex/EventBase.cpp notifex/EventBase.h notifex/Demultiplexer.h notifex/Epoller.cpp notifex/Epoller.h notifex/Event.cpp notifex/Event.h notifex/Timer.cpp notifex/Timer.h notifex/ThreadPool.h notifex/Clock.cpp notifex/Clock.h notifex/RIO.h notifex/RIO.cpp notifex/Util.h notifex/TCPListener.cpp notifex/TCPListener.h example/test/test_cli.cpp notifex/Socket.cpp notifex/Socket.h notifex/TCPConnection.cpp notifex/TCPConnection.h notifex/Callbacks.h notifex/Types.h notifex/Channel.cpp notifex/Channel.h notifex/Demultiplexer.cpp notifex/DefaultDemultiplexer.cpp notifex/TCPServer.cpp notifex/TCPServer.h notifex/Buffer.cpp notifex/Buffer.h notifex/Endian.h notifex/SockAddress.cpp notifex/SockAddress.h)