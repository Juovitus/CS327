cmake_minimum_required(VERSION 3.21)
project(Poke C)

set(CMAKE_C_STANDARD 99)

add_executable(Poke main.c heap.c heap.h)
