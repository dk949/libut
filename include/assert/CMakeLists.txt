cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_assert INTERFACE)
target_sources(libut_target_assert PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ut/assert/assert.hpp
)

message(STATUS "libut: adding UT::assert")
add_library(UT::assert ALIAS libut_target_assert)
