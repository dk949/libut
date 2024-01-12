cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_print INTERFACE)
target_include_directories(libut_target_print INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
message(STATUS "libut: adding UT::print")
add_library(UT::print ALIAS libut_target_print)
