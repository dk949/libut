cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_check INTERFACE)
target_sources(libut_target_check PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ut/check/check.hpp
)

message(STATUS "libut: adding UT::check")
add_library(UT::check ALIAS libut_target_check)
