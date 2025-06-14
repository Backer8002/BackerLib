cmake_minimum_required(VERSION 3.13)

project(BackerLibTypes VERSION 0.1)


set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED 23)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED 14)

include_directories(BackerLibTypes INTERFACE "${PROJECT_SOURCE_DIR}/../include")
add_library(BackerLibTypes arrayList.c bitsets.c extendedMath.c hashMap.c hashMapCuckoo.c queue.c strings.c)
