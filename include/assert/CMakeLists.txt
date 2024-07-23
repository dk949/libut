cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_assert INTERFACE)
target_include_directories(libut_target_assert INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
message(STATUS "libut: adding UT::assert")
add_library(UT::assert ALIAS libut_target_assert)
