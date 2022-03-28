cmake_minimum_required(VERSION 3.21)
project(Poke C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(Poke
        heap.c
        heap.h
        main.c)
find_package( Curses REQUIRED )
target_link_libraries( Poke ${CURSES_LIBRARIES} )
include_directories( ${CURSES_INCLUDE_DIRS} )