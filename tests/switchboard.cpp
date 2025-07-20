#include <catch.hpp>
#include <catch2/catch_test_macros.hpp>
#include <ut/switchboard/switchboard.hpp>

UT_ARGS(MyArgs) {
    UT_ARG(bool) force {'f', "--force", Help {"Force some behaviour"}};
};

UT_ARGS(ChildArgs, MyArgs) {
    UT_ARG(bool) force_force {"--force-force", Help {"Really force some behaviour"}};
    UT_ARG(std::string_view) output {'o', Metavar {"FILE"}, Default {"a.out"}};
};

TEST_CASE("Switch board smoke test", "[switchboard]") { }

TEST_CASE("Switch board out of order", "[switchboard]") { }
