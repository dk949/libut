cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_defer INTERFACE)
target_sources(libut_target_defer PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ut/defer/defer.hpp
)

message(STATUS "libut: adding UT::defer")
add_library(UT::defer ALIAS libut_target_defer)
