cmake_minimum_required(VERSION 3.25)

project(BackerLibEvent)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic")


add_library(BackerLibEvent STATIC Event_Internal.h Event.c)

target_include_directories(BackerLibEvent INTERFACE "${PROJECT_SOURCE_DIR}/../include")
add_dependencies(BackerLibEvent BackerLibConcurrency BackerLibTypes)
target_link_libraries(BackerLibEvent LINK_PUBLIC BackerLibConcurrency BackerLibTypes)
