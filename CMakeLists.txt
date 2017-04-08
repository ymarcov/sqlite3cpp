cmake_minimum_required(VERSION 2.6)

get_directory_property(hasParent PARENT_DIRECTORY)

if (hasParent)
    set(sqlite3cpp_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_compile_options(-std=c++14 -fPIC -O3 -Werror -Wall -Wpedantic)

project(sqlite3cpp)
add_library(sqlite3cpp SHARED Sqlite.cc)

project(sqlite3cpp_stati)
add_library(sqlite3cpp_static STATIC Sqlite.cc)
