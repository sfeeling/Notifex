cmake_minimum_required(VERSION 3.10)
project(Notifex)

set(CMAKE_CXX_STANDARD 14)

add_library(notifex SHARED notifex/notifex.h notifex/EventBase.cpp notifex/EventBase.h notifex/Demultiplexer.h notifex/Epoller.cpp notifex/Epoller.h notifex/Event.cpp notifex/Event.h notifex/Timer.cpp notifex/Timer.h notifex/ThreadPool.cpp notifex/ThreadPool.h notifex/Clock.cpp notifex/Clock.h notifex/RIO.h notifex/RIO.cpp)