cmake_minimum_required(VERSION 3.21)
project(libut LANGUAGES CXX)

add_library(libut_target_ptr_containers INTERFACE)
target_sources(libut_target_ptr_containers PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ut/ptr_containers/valueptr.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ut/ptr_containers/ownptrvec.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ut/ptr_containers/ptrvecview.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ut/ptr_containers/container_base.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ut/ptr_containers/template_helpers.hpp
)
message(STATUS "libut: adding UT::ptr_containers")
add_library(UT::ptr_containers ALIAS libut_target_ptr_containers)
