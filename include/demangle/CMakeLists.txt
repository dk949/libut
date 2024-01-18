cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_demangle INTERFACE)
target_include_directories(libut_target_demangle INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
message(STATUS "libut: adding UT::demangle")
add_library(UT::demangle ALIAS libut_target_demangle)
