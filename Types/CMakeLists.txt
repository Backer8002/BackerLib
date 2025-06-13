cmake_minimum_required(VERSION 3.10)

project(BackerLibTypes VERSION 0.1)


set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED 23)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED 14)

include_directories(BackerLibTypes PUBLIC "${PROJECT_SOURCE_DIR}/../include")
add_library(BackerLibTypes STATIC arrayList.c bitsets.c extendedMath.c hashMap.c hashMapNew.c queue.c strings.c)
target_link_libraries(BackerLibTypes "BackerLibTypes.lib")
