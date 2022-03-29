cmake_minimum_required(VERSION 3.21)
project(Poke)

set(CMAKE_C_STANDARD 11)

add_executable(Poke
        heap.c
        heap.h
        main.cpp)
set_target_properties(Poke PROPERTIES LINKER_LANGUAGE CXX)
find_package( Curses REQUIRED )
target_link_libraries( Poke ${CURSES_LIBRARIES} )
include_directories( ${CURSES_INCLUDE_DIRS} )