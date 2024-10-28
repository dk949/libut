cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_sv_to_num INTERFACE)
target_sources(libut_target_sv_to_num PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ut/sv_to_num/sv_to_num.hpp
)

message(STATUS "libut: adding UT::sv_to_num")
add_library(UT::sv_to_num ALIAS libut_target_sv_to_num)
