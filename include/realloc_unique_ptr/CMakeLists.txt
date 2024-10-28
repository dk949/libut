cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_realloc_unique_ptr INTERFACE)
target_sources(libut_target_realloc_unique_ptr PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ut/realloc_unique_ptr/realloc_unique_ptr.hpp
)

message(STATUS "libut: adding UT::realloc_unique_ptr")
add_library(UT::realloc_unique_ptr ALIAS libut_target_realloc_unique_ptr)
