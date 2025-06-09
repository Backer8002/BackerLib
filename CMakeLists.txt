cmake_minimum_required(VERSION 3.10)

project(BackerLib VERSION 0.1)

configure_file(BackerLibVersion.h.in BackerLibVersion.h)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED 17)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED 14)

add_subdirectory(basicFunctions)
add_subdirectory(BackerLoggingHandler)