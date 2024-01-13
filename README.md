# libut

[![CMake build and test](https://github.com/dk949/libut/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/dk949/libut/actions/workflows/cmake-multi-platform.yml)

    Various random files I've been saving as gists, all submoduled in one big repo.

This is a collection of C++ headers (header only libraries?) for doing useful
things. Such as printing human readable type names or converting `string_view`s
to numbers.

All files are in individual directories under `include`.

Documentation can be found at the top of each file.

## Getting `libut`

The easiest way to include `libut` in your `CMake` project is with
`fetch_content`.

Simply add the following to your `CMakeLists.txt`:

```cmake
include(FetchContent)
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

* `UT::check`
* `UT::add_noexcept`
* `UT::smallfn`
* `UT::sv_to_num`
* `UT::realloc_unique_ptr`
* `UT::demangle`
* `UT::constexpr_hash`
* `UT::pack_loops`
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
ctest --test-dir build/tests
```

## License

Each header file has a license (usually at the bottom). For files which don't
(such as scripts and `CMakeLists.txt`s), see `LICENSE` file in the root of this
repo.
