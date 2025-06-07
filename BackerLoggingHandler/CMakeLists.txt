cmake_minimum_required(VERSION 3.10)

project(BackerLoggingHandler)

include_directories(BackerLoggingHandler PUBLIC "${PROJECT_SOURCE_DIR}/../include")

add_library(BackerLoggingHandler STATIC loggingMain.c)


target_link_libraries(BackerLoggingHandler "BackerLoggingHandler.lib")
