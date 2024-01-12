cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_add_noexcept INTERFACE)
target_include_directories(libut_target_add_noexcept INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
message(STATUS "libut: adding UT::add_noexcept")
add_library(UT::add_noexcept ALIAS libut_target_add_noexcept)
