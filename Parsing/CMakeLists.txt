cmake_minimum_required(VERSION 3.25)

project(BackerLibParsing LANGUAGES C VERSION 0.1)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)


add_library(BackerLibParsing STATIC
        Json.h
        JsonParser.c
        ${PROJECT_SOURCE_DIR}/../include/BackerLibParsing.h
        JsonWrite.c
        JsonObjectManipulations.c)

target_include_directories(BackerLibParsing INTERFACE "${PROJECT_SOURCE_DIR}/../include")
add_dependencies(BackerLibParsing BackerLibTypes BackerLibEvent BackerLibConcurrency)
find_library(MATH_LIB m)
if(MATH_LIB)
    target_link_libraries(BackerLibParsing LINK_PUBLIC ${MATH_LIB} BackerLibTypes BackerLibEvent BackerLibConcurrency)
else ()
    target_link_libraries(BackerLibParsing LINK_PUBLIC BackerLibTypes BackerLibEvent BackerLibConcurrency)
endif ()