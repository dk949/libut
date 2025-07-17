include(compiler_warnings.cmake)
find_package(Catch2 3 REQUIRED)
include(CTest)
include(Catch)

set(cxx_17_files
    ${CMAKE_CURRENT_SOURCE_DIR}/add_noexcept.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/check.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/constexpr_hash.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/defer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pack_loops.cpp
)
set(cxx_20_files
    ${cxx_17_files}
    ${CMAKE_CURRENT_SOURCE_DIR}/change_observer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/copy_traits.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/demangle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mt_queue.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ownptrvec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pair.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ptrvecview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/resource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/static_string.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sv_to_num.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/template_helpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/trim.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/valueptr.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/switchboard.cpp
)

set(cxx_23_files
    ${cxx_20_files}
    ${CMAKE_CURRENT_SOURCE_DIR}/err.cpp
)

function(setup_target target)
    target_link_libraries(${target} PRIVATE UT::all)
    set_target_warnings(${target})
    target_link_libraries(${target} PRIVATE Catch2::Catch2WithMain)
    target_precompile_headers(${target} PRIVATE <catch.hpp>)
    if(NOT MSVC)
        target_compile_options(${target} PUBLIC "-fsanitize=address" "-Og" "-g")
        target_link_options(${target} PUBLIC "-fsanitize=address" "-lg")
    else()
        target_compile_options(${target} PUBLIC "/Zc:__cplusplus")
    endif()
    catch_discover_tests(${target})
endfunction()

add_executable(libut_tests_17 ${cxx_17_files})
add_executable(libut_tests_20 ${cxx_20_files})
add_executable(libut_tests_23 ${cxx_23_files})

target_compile_features(libut_tests_17 PUBLIC cxx_std_17)
target_compile_features(libut_tests_20 PUBLIC cxx_std_20)
target_compile_features(libut_tests_23 PUBLIC cxx_std_23)

setup_target(libut_tests_17)
setup_target(libut_tests_20)
setup_target(libut_tests_23)
