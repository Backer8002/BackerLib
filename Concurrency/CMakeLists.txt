cmake_minimum_required(VERSION 3.25)

project(BackerLibConcurrency LANGUAGES C VERSION 0.1)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(BackerLibConcurrency STATIC
        ThreadPool.c
        ThreadPool.h
        ConcurrencyDefines.h
        ../include/BackerLibConcurrency.h)

target_include_directories(BackerLibConcurrency BEFORE INTERFACE "${PROJECT_SOURCE_DIR}/../include")
target_include_directories(BackerLibConcurrency BEFORE PUBLIC "${PROJECT_BINARY_DIR}/..")

add_dependencies(BackerLibConcurrency BackerLibTypes)
target_link_libraries(BackerLibConcurrency LINK_PUBLIC BackerLibTypes)