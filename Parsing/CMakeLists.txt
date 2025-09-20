cmake_minimum_required(VERSION 3.25)

project(BackerLibParsing LANGUAGES C VERSION 0.1)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(BackerLibParsing STATIC
            JSONParser.h
            JSONParser.c)

target_include_directories(BackerLibParsing INTERFACE "../include")