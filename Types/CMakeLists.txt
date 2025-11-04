cmake_minimum_required(VERSION 3.25)

project(BackerLibTypes LANGUAGES C VERSION 0.1)


set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(BackerLibTypes INTERFACE "${PROJECT_SOURCE_DIR}/../include")
add_library(BackerLibTypes STATIC
        BL_ArrayList.h
        BL_ArrayList.c
        Bitsets.c
        HashMapClosed.c
        BL_Queue.c
        BL_Queue.h
        BackerStrings.c
        BackerStrings.h
        BL_Container.c
        BL_Container.h
        BL_DynamicContainer.c
        BL_DynamicContainer.h
        BL_UnorderedContainer.c
        BL_UnorderedContainer.h
        BL_Heap.c
        BL_Heap.h
        BL_Trie.c
        BL_Trie.h
        HashMap.h
        HashMapPublicShared.c
        )

