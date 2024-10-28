cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_pack_loops INTERFACE)
target_sources(libut_target_pack_loops PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ut/pack_loops/pack_loops.hpp
)

message(STATUS "libut: adding UT::pack_loops")
add_library(UT::pack_loops ALIAS libut_target_pack_loops)
