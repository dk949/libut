# libut

[![CMake build and test](https://github.com/dk949/libut/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/dk949/libut/actions/workflows/cmake-multi-platform.yml)

    Various random files I've been saving as gists, all submoduled in one big repo.

This is a collection of C++ headers (header only libraries?) for doing useful
things. Such as printing human readable type names or converting `string_view`s
to numbers.

All files are in individual directories under `include`.

Documentation can be found at the top of each file.

## Testing

Test can be ran using Catch2

```sh
git clone https://github.com/Microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh -disableMetrics
./vcpkg/vcpkg install catch2
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
./build/tests/libut_tests
# or
ctest --test-dir build/tests
```

## License

Each header file has a license (usually at the bottom). For files which don't
(such as scripts and `CMakeLists.txt`s), see `LICENSE` file in the root of this
repo.
