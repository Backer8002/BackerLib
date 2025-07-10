cmake_minimum_required(VERSION 3.13)

project(BackerLibEvent)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED 23)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED 14)

include_directories(BackerLibEvent INTERFACE "${PROJECT_SOURCE_DIR}/../include")

add_library(BackerLibEvent STATIC eventMain.c)

