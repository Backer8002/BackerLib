cmake_minimum_required(VERSION 3.25)

project(BackerLibLogging LANGUAGES C VERSION 0.1)

add_library(BackerLibLogging STATIC Logging.c Logging.h Logging_Internal.h)

target_include_directories(BackerLibLogging BEFORE INTERFACE "${PROJECT_SOURCE_DIR}/../include")

add_dependencies(BackerLibLogging BackerLibConcurrency)

target_link_libraries(BackerLibLogging BackerLibConcurrency)
