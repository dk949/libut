cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_print INTERFACE)
target_sources(libut_target_print PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ut/print/print.hpp
)

message(STATUS "libut: adding UT::print")
add_library(UT::print ALIAS libut_target_print)
