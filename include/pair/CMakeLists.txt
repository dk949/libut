cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_pair INTERFACE)
target_sources(libut_target_pair PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ut/pair/pair.hpp
)

message(STATUS "libut: adding UT::pair")
add_library(UT::pair ALIAS libut_target_pair)
