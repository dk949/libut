#include <ut/breakpoint/breakpoint.hpp>

[[maybe_unused]]
void dummy() {
    // Don't really know how to test this (maybe signal handler?)
    // This is just a test that this compiles
    UT_BREAKPOINT();
}
