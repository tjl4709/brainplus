cmake_minimum_required(VERSION 3.20)
project(brainplus)

set(CMAKE_CXX_STANDARD 14)

add_executable(brainplus main.cpp Lexer.cpp Lexer.h)
