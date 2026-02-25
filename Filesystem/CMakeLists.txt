cmake_minimum_required(VERSION 3.25)

project(BackerLibFilesystem LANGUAGES C VERSION 0.1)

set(targets "Filesystem.h" "Utils.c")

message(CHECK_START "Choosing compatibe os filesystem api")
if(UNIX)
set(targets ${targets} "FilesystemPosix.c")
message(CHECK_PASS "Using posix api")
elseif(WIN32)
set(targets ${targets} "FilesystemWindows.c")
message(CHECK_PASS "Using windows api")
else()
message(CHECK_FAIL "Unsupported operating system")
message(FATAL_ERROR "Unable to process without selected api")
endif()

add_library(BackerLibFilesystem STATIC ${targets})

target_include_directories(BackerLibFilesystem BEFORE INTERFACE "${PROJECT_SOURCE_DIR}/../include")

add_dependencies(BackerLibFilesystem BackerLibTypes)

target_link_libraries(BackerLibFilesystem BackerLibTypes)
