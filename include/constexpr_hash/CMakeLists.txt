cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_constexpr_hash INTERFACE)
target_sources(libut_target_constexpr_hash PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ut/constexpr_hash/constexpr_hash.hpp
)

message(STATUS "libut: adding UT::constexpr_hash")
add_library(UT::constexpr_hash ALIAS libut_target_constexpr_hash)
