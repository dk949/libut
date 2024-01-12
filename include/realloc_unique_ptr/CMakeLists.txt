cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_realloc_unique_ptr INTERFACE)
target_include_directories(libut_target_realloc_unique_ptr INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
message(STATUS "libut: adding UT::realloc_unique_ptr")
add_library(UT::realloc_unique_ptr ALIAS libut_target_realloc_unique_ptr)
