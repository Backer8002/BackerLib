cmake_minimum_required(VERSION 3.25)

project(BackerLibTypes LANGUAGES C VERSION 0.1)


set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(BackerLibTypes INTERFACE "${PROJECT_SOURCE_DIR}/../include")
add_library(BackerLibTypes STATIC
        ArrayList.h
        ArrayList.c
        Bitsets.c
        HashMapClosed.c
        Queue.c
        Queue.h
        BackerStrings.c
        BackerStrings.h
        Container.c
        Container.h
        DynamicContainer.c
        DynamicContainer.h
        UnorderedContainer.c
        UnorderedContainer.h
        Heap.c
        Heap.h
        Trie.c
        Trie.h
        HashMap.h
        HashMapPublicShared.c
        )

