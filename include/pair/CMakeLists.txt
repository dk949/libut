cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_pair INTERFACE)
target_include_directories(libut_target_pair INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
message(STATUS "libut: adding UT::pair")
add_library(UT::pair ALIAS libut_target_pair)
