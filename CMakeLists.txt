cmake_minimum_required(VERSION 3.10)
project(Notifex)

set(CMAKE_CXX_STANDARD 11)

add_library(notifex SHARED notifex.cpp notifex.h)