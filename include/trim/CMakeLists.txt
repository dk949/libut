cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_trim INTERFACE)
target_sources(libut_target_trim PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ut/trim/trim.hpp
)

message(STATUS "libut: adding UT::trim")
add_library(UT::trim ALIAS libut_target_trim)
