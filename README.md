# libut

[![CMake build and test](https://github.com/dk949/libut/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/dk949/libut/actions/workflows/cmake-multi-platform.yml)

    Various useful bits of c++

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

* `UT::ptr_containers`
* `UT::check`
* `UT::pair`
* `UT::add_noexcept`
* `UT::sv_to_num`
* `UT::copy_traits`
* `UT::realloc_unique_ptr`
* `UT::assert`
* `UT::resource`
* `UT::demangle`
* `UT::constexpr_hash`
* `UT::trim`
* `UT::pack_loops`
* `UT::static_string`
* `UT::mt_queue`
* `UT::curry`
* `UT::defer`
* `UT::print`

<!-- AUTOGEN_END -->

A special target `UT::all` is also available, combining all of the above.

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
