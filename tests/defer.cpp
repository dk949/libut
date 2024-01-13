#include <catch.hpp>
#include <ut/defer/defer.hpp>

#include <numeric>
#include <vector>

using namespace Catch::Matchers;

TEST_CASE("Defer", "[defer]") {
    std::vector<int> vec;
    {
        defer {
            vec.push_back(6);
        };
        vec.push_back(0);
        {
            defer {
                vec.push_back(3);
            };
            vec.push_back(1);
            defer {
                vec.push_back(2);
            };
        }
        defer {
            vec.push_back(5);
        };
        vec.push_back(4);
    }

    // Could use std::is_sorted, but using the matcher catch will print the contents of vec if it doesn't match
    std::vector<int> expected(7);
    std::iota(expected.begin(), expected.end(), 0);
    REQUIRE_THAT(vec, Equals(expected));
}
