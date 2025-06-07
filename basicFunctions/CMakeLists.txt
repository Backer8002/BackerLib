cmake_minimum_required(VERSION 3.10)

project(basicFunctions VERSION 0.1)


set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED 23)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED 14)

include_directories(basicFunctions PUBLIC "${PROJECT_SOURCE_DIR}/../include")
add_library(basicFunctions STATIC arrayList.c bitsets.c extendedMath.c hashMap.c queue.c strings.c)
target_link_libraries(basicFunctions "basicFunctions.lib")
