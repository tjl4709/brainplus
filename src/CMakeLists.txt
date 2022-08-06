cmake_minimum_required(VERSION 3.20)
project(brainplus)

set(CMAKE_CXX_STANDARD 14)

add_executable(brainplus enums.h Lexer.h Lexer.cpp ASTNodes.h Parser.cpp Parser.h main.cpp)
