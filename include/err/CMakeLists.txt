cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_err INTERFACE)
target_sources(libut_target_err PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ut/err/err.hpp
)

message(STATUS "libut: adding UT::err")
add_library(UT::err ALIAS libut_target_err)
    