#include <catch.hpp>
#include <catch2/catch_test_macros.hpp>
#include <ut/switchboard/switchboard.hpp>

TEST_CASE("Switch board", "[switchboard]") {

    using namespace ut::sw;
    using force = Arg<bool, "-f", "--force", desc<"kjhk">>;
    using output = Arg<std::string, "-o", name<"output">, desc<"desc">>;
    using input = Arg<std::optional<std::string>, "-i", name<"input">, desc<"aaaaaa">>;
    ut::sw::ArgParser<force, output, input, Arg<bool, "-v", "--verbose">> parser;
}
