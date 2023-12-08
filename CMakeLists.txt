cmake_minimum_required(VERSION 3.5)

project(localchat C)

add_executable(chatserver src/tcpserver.c)
add_executable(chatclient src/tcpclient.c)
