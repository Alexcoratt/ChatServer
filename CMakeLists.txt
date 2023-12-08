cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(localchat CXX)

add_executable(chatserver src/chatserver.cpp)
add_executable(chatclient src/chatclient.cpp)
