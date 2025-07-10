cmake_minimum_required(3.13)


set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED 17)

add_library(BackerLibMemory STATIC memoryAllocators.c)

target_include_directories(BackerLibMemory INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")