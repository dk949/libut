cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_demangle INTERFACE)
target_sources(libut_target_demangle PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ut/demangle/demangle.hpp
)

message(STATUS "libut: adding UT::demangle")
add_library(UT::demangle ALIAS libut_target_demangle)
