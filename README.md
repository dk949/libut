# libut

[![CMake build and test](https://github.com/dk949/libut/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/dk949/libut/actions/workflows/cmake-multi-platform.yml)

    Various useful bits of C++

This is header only C++ library for doing useful things. Such as printing human
readable type names, converting `string_view`s to numbers. See the [Currently
available targets](#currently-available-targets) section for detail.

All files are in individual directories under `include`.

Documentation can be found at the top of each file.

## Getting `libut`

The easiest way to include `libut` in your `CMake` project is with
`fetch_content`.

Simply add the following to your `CMakeLists.txt`:

```cmake
include(FetchContent)
# Set the directory to download dependencies to. Avoids putting them in $buildDir
set(FETCHCONTENT_BASE_DIR "${PROJECT_SOURCE_DIR}/_deps")
FetchContent_Declare(
  libut
  GIT_REPOSITORY https://github.com/dk949/libut/
  GIT_TAG trunk # alternatively you can use a hash to pin exact venison to use
)
FetchContent_MakeAvailable(libut)

# assuming MY_TARGET is a valid target (executable or library)

target_link_libraries(MY_TARGET UT::target_name)
```

### Currently available targets

<!-- AUTOGEN_BEGIN -->

* `UT::ptr_containers`: containers for storing owning pointers
* `UT::check`: type which prints when it is constructed/copied/moved/destructed
* `UT::pair`: a better `std::pair` implementation
* `UT::switchboard`: composable and type-safe CLI parser
* `UT::err`: convenient error type for use with std::expected
* `UT::add_noexcept`: add the `noexcept` specifier to a function type
* `UT::change_observer`: wrapper class that invokes callbacks when it's held value changes
* `UT::traced_error`: an exception type that automatically stores a C++23 standard stacktrace
* `UT::sv_to_num`: convert `std::string_view` to a number
* `UT::copy_traits`: copy cvref qualifiers from one type to another
* `UT::realloc_unique_ptr`: `realloc` functionality for `std::uniqur_ptr`
* `UT::assert`: various assertion macros
* `UT::resource`: a more general version of `std::unique_ptr` that operates on values
* `UT::demangle`: convert types into human readable strings
* `UT::constexpr_hash`: a hashing function usable in `constexpr` context
* `UT::trim`: trim characters from start and end of a `std::string_view`
* `UT::pack_loops`: loop over variadic template parameters (either values or types)
* `UT::asis`: Provides a more convenient way to work with different kinds of dynamically polymorphic values.
* `UT::static_string`: compile time known string usable as a template parameter
* `UT::breakpoint`: Portable(ish) breakpoint macros
* `UT::mt_queue`: thread safe FIFO queue
* `UT::curry`: create a curried function out of a regular function
* `UT::defer`: a macro to defer execution until the scope ends
* `UT::print`: macros for printing using `std::format` in c++20
* `UT::overload`: creates a function object merging several other function objects as an overload set

<!-- AUTOGEN_END -->

A special target `UT::ut` is also available, combining all of the above.

## Testing

Test can be ran using Catch2

```sh
git clone https://github.com/Microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh -disableMetrics
./vcpkg/vcpkg install catch2
cmake --preset local
cmake --build build
./build/tests/libut_tests
# or
ctest --output-on-failure --test-dir build/tests
```

_NOTE:_ you can use the `local` preset to build with
[ninja](https://ninja-build.org/), or `default` to build with the default
generator (e.g. "Unix Makefiles" on Linux)

## License

The files included as gists, the license is included in the file (usually at the
bottom). For all other files, see `LICENSE` in the root of this repo.
