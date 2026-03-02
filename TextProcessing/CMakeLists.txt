cmake_minimum_required(VERSION 3.25)

project(BackerLibTextProcessing VERSION 0.1 LANGUAGES C)

add_library(BackerLibTextProcessing UnicodeString.c UnicodeString.h Conversions.c Conversions.h)

add_dependencies(BackerLibTextProcessing BackerLibTypes)

target_include_directories(BackerLibTextProcessing INTERFACE ${PROJECT_SOURCE_DIR}/../include)

target_link_libraries(BackerLibTextProcessing BackerLibTypes)
