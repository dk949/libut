include(compiler_warnings.cmake)

file(GLOB_RECURSE TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_executable(libut_tests ${TEST_FILES})
target_link_libraries(libut_tests PRIVATE UT::all)
set_target_warnings(libut_tests)

find_package(Catch2 3 REQUIRED)
target_link_libraries(libut_tests PRIVATE Catch2::Catch2WithMain)
target_include_directories(libut_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

include(CTest)
include(Catch)
catch_discover_tests(libut_tests)
