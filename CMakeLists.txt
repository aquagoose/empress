cmake_minimum_required(VERSION 4.0)
project(empress C)

set(CMAKE_C_STANDARD 99)

add_library(empress SHARED
        include/empress/empress.h
        src/empress.c)

target_include_directories(empress PUBLIC include)

target_link_libraries(empress PRIVATE systemd)