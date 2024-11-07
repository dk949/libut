cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_static_string INTERFACE)
target_sources(libut_target_static_string PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ut/static_string/static_string.hpp
)

message(STATUS "libut: adding UT::static_string")
add_library(UT::static_string ALIAS libut_target_static_string)
