cmake_minimum_required(VERSION 3.25)

project(BackerLib VERSION 0.1)

configure_file(BackerLibVersion.h.in BackerLibVersion.h)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_definitions(BL_LOGGING_INFO BL_LOGGING_TRACE BL_LOGGING_DEBUG BL_LOGGING_WARN BL_LOGGING_ERROR BL_LOGGING_CRITICAL)

if (MSVC)
    add_compile_options(/Wall /JMC /experimental:c11atomics)
else ()
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion)
endif ()

include_directories(BEFORE PUBLIC ${CMAKE_BINARY_DIR} "${PROJECT_SOURCE_DIR}/include")

add_subdirectory(Types)
add_subdirectory(Math)
add_subdirectory(Event)
add_subdirectory(Concurrency)
add_subdirectory(Parsing)
add_subdirectory(Logging)
add_subdirectory(TextProcessing)
