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

target_link_libraries(MY_TARGET ut::target_name)
```

### Currently available targets

<!-- AUTOGEN_BEGIN -->

* `ut::ptr_containers`: containers for storing owning pointers
* `ut::check`: type which prints when it is constructed/copied/moved/destructed
* `ut::pair`: a better `std::pair` implementation
* `ut::err`: convenient error type for use with std::expected
* `ut::add_noexcept`: add the `noexcept` specifier to a function type
* `ut::args`: CLI parser
* `ut::change_observer`: wrapper class that invokes callbacks when it's held value changes
* `ut::traced_error`: an exception type that automatically stores a C++23 standard stacktrace
* `ut::sv_to_num`: convert `std::string_view` to a number
* `ut::copy_traits`: copy cvref qualifiers from one type to another
* `ut::realloc_unique_ptr`: `realloc` functionality for `std::uniqur_ptr`
* `ut::assert`: various assertion macros
* `ut::resource`: a more general version of `std::unique_ptr` that operates on values
* `ut::demangle`: convert types into human readable strings
* `ut::constexpr_hash`: a hashing function usable in `constexpr` context
* `ut::trim`: trim characters from start and end of a `std::string_view`
* `ut::pack_loops`: loop over variadic template parameters (either values or types)
* `ut::dirs`: get standard directories for data/cache/logs/etc. on different platforms
* `ut::asis`: Provides a more convenient way to work with different kinds of dynamically polymorphic values.
* `ut::spawn`: simple process spawning (currently Linux only)
* `ut::static_string`: compile time known string usable as a template parameter
* `ut::breakpoint`: Portable(ish) breakpoint macros
* `ut::mt_queue`: thread safe FIFO queue
* `ut::curry`: create a curried function out of a regular function
* `ut::defer`: a macro to defer execution until the scope ends
* `ut::print`: macros for printing using `std::format` in c++20
* `ut::overload`: creates a function object merging several other function objects as an overload set

<!-- AUTOGEN_END -->

A special target `ut::ut` is also available, combining all of the above.

### Using with `find_package`

If you prefer, you can install the library and import it into a project via
`find_package` like so:


```sh
git clone github.com/dk949/libut
cd libut
# use cmake --list-presets for a list of available presets
cmake --preset local -DCMAKE_INSTALL_PREFIX=./libut-install
cmake --install build
```

Then in your cmake file:

```cmake
set(ENV{ut_DIR} /path/to/libut/libut-install)
find_package(ut REQUIRED)
target_link_libraries(MY_TARGET ut::ut_target_name)
```

> [!NOTE]
> When using find_package, all target names above (except ut::ut) are prefixed
> with`ut_`.
>
> The prefixed targets are also available when using `FetchContent`, so for
> a config that could use either, use the prefixed targets.

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
