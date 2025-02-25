cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_change_observer INTERFACE)
target_sources(libut_target_change_observer PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ut/change_observer/change_observer.hpp
)

message(STATUS "libut: adding UT::change_observer")
add_library(UT::change_observer ALIAS libut_target_change_observer)

