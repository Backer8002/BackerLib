cmake_minimum_required(VERSION 3.25)

project(BackerLibTypes VERSION 0.1)


set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic")

include_directories(BackerLibTypes INTERFACE "${PROJECT_SOURCE_DIR}/../include")
add_library(BackerLibTypes STATIC
        Bitsets.c
        HashMap.c
        HashMapCuckoo.c
        Queue.c
        BackerStrings.c
        Container.c
        DynamicContainer.c
        UnorderedContainer.c
        Heap.c
        Trie.c
        Trie.h)

