cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_breakpoint INTERFACE)
target_sources(libut_target_breakpoint PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ut/breakpoint/breakpoint.hpp
)

message(STATUS "libut: adding UT::breakpoint")
add_library(UT::breakpoint ALIAS libut_target_breakpoint)
    