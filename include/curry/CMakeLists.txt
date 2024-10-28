cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_curry INTERFACE)
target_sources(libut_target_curry PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ut/curry/curry.hpp
)

message(STATUS "libut: adding UT::curry")
add_library(UT::curry ALIAS libut_target_curry)
