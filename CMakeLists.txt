cmake_minimum_required(VERSION 3.25)

project(BackerLib VERSION 0.1)

configure_file(BackerLibVersion.h.in BackerLibVersion.h)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

if (MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Wall /JMC /experimental:c11atomics")
else ()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic")
endif ()

include_directories(BEFORE PUBLIC ${CMAKE_BINARY_DIR} "${PROJECT_SOURCE_DIR}/include")

add_subdirectory(Types)
add_subdirectory(Math)
add_subdirectory(Event)
add_subdirectory(Concurrency)


