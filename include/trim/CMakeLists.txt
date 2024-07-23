cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_trim INTERFACE)
target_include_directories(libut_target_trim INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
message(STATUS "libut: adding UT::trim")
add_library(UT::trim ALIAS libut_target_trim)
