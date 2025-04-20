cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_overload INTERFACE)
target_sources(libut_target_overload PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ut/overload/overload.hpp
)

message(STATUS "libut: adding UT::overload")
add_library(UT::overload ALIAS libut_target_overload)
    