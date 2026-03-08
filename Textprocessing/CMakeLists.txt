cmake_minimum_required(VERSION 3.25)

project(BackerLibTextprocessing VERSION 0.1 LANGUAGES C)

add_library(BackerLibTextprocessing UnicodeString.c UnicodeString.h Conversions.c Conversions.h)

add_dependencies(BackerLibTextprocessing BackerLibTypes)

target_include_directories(BackerLibTextprocessing INTERFACE ${PROJECT_SOURCE_DIR}/../include)

target_link_libraries(BackerLibTextprocessing BackerLibTypes)
