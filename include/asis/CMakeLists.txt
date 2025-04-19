cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_asis INTERFACE)
target_sources(libut_target_asis PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ut/asis/asis.hpp
)

message(STATUS "libut: adding UT::asis")
add_library(UT::asis ALIAS libut_target_asis)
    