cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_resource INTERFACE)
target_include_directories(libut_target_resource INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
message(STATUS "libut: adding UT::resource")
add_library(UT::resource ALIAS libut_target_resource)
