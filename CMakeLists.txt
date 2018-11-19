cmake_minimum_required(VERSION 3.10)
project(Notifex)

set(CMAKE_CXX_STANDARD 14)

add_library(notifex SHARED notifex.h EventBase.cpp EventBase.h Demultiplexer.h Epoller.cpp Epoller.h Event.cpp Event.h Timer.cpp Timer.h)