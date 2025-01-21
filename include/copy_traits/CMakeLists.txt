cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_copy_traits INTERFACE)
target_sources(libut_target_copy_traits PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ut/copy_traits/copy_traits.hpp
)

message(STATUS "libut: adding UT::copy_traits")
add_library(UT::copy_traits ALIAS libut_target_copy_traits)
