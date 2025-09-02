cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_spawn INTERFACE)
target_sources(libut_target_spawn PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ut/spawn/spawn.hpp
)
message(STATUS "libut: adding UT::spawn")
add_library(UT::spawn ALIAS libut_target_spawn)
