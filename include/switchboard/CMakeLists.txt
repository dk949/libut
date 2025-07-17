cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_switchboard INTERFACE)
target_sources(libut_target_switchboard PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ut/switchboard/switchboard.hpp
)

message(STATUS "libut: adding UT::switchboard")
add_library(UT::switchboard ALIAS libut_target_switchboard)

