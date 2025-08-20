cmake_minimum_required(VERSION 3.25)

project(BackerLibEvent)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic")

include_directories(BackerLibEvent INTERFACE "${PROJECT_SOURCE_DIR}/../include")

add_library(BackerLibEvent STATIC eventMain.c
        logFunctions.c)

