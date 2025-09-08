cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_traced_error INTERFACE)
target_sources(libut_target_traced_error PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ut/traced_error/traced_error.hpp
)

message(STATUS "libut: adding UT::traced_error")
add_library(UT::traced_error ALIAS libut_target_traced_error)
    