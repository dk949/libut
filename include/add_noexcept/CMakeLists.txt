cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_add_noexcept INTERFACE)
target_sources(libut_target_add_noexcept PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ut/add_noexcept/add_noexcept.hpp
)

message(STATUS "libut: adding UT::add_noexcept")
add_library(UT::add_noexcept ALIAS libut_target_add_noexcept)
