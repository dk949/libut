include(compiler_warnings.cmake)

set(TEST_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/add_noexcept.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/change_observer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/check.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/constexpr_hash.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/copy_traits.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/defer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/demangle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_queue.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ownptrvec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pack_loops.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pair.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ptrvecview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/resource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/static_string.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sv_to_num.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/template_helpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/trim.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/valueptr.cpp
)
add_executable(libut_tests ${TEST_FILES})
target_link_libraries(libut_tests PRIVATE UT::all)
set_target_warnings(libut_tests)

find_package(Catch2 3 REQUIRED)
target_link_libraries(libut_tests PRIVATE Catch2::Catch2WithMain)
target_precompile_headers(libut_tests PRIVATE <catch.hpp>)

if(NOT MSVC)
    target_compile_options(libut_tests PUBLIC "-fsanitize=address" "-Og" "-g")
    target_link_options(libut_tests PUBLIC "-fsanitize=address" "-lg")
else()
    target_compile_options(libut_tests PUBLIC "/Zc:__cplusplus")
endif()

include(CTest)
include(Catch)
catch_discover_tests(libut_tests)
