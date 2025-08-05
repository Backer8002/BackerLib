cmake_minimum_required(VERSION 3.25)

project(BackerLib VERSION 0.1)

configure_file(BackerLibVersion.h.in BackerLibVersion.h)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic")

add_subdirectory(Types)
add_subdirectory(Math)
add_subdirectory(Event)