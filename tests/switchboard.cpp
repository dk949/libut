#include <catch.hpp>
#include <catch2/catch_test_macros.hpp>
#include <ut/switchboard/switchboard.hpp>

TEST_CASE("Switch board smoke test", "[switchboard]") {

    using namespace ut::sw;
    using force = Arg<bool, "-f", "--force", desc<"force action">>;
    using output = Arg<std::string, "-o", name<"output">>;
    constexpr ArgParser<force, output, Arg<bool, "-v", "--verbose">> parser;

    STATIC_REQUIRE(std::get<0>(parser.args).short_flag == "-f");
    STATIC_REQUIRE(std::get<0>(parser.args).long_flag == "--force");
    STATIC_REQUIRE(std::get<0>(parser.args).name == "force");
    STATIC_REQUIRE(std::get<0>(parser.args).description == "force action");

    STATIC_REQUIRE(std::get<1>(parser.args).short_flag == "-o");
    STATIC_REQUIRE(!std::get<1>(parser.args).long_flag);
    STATIC_REQUIRE(std::get<1>(parser.args).name == "output");
    STATIC_REQUIRE(!std::get<1>(parser.args).description);
}

TEST_CASE("Switch board out of order", "[switchboard]") {
    using namespace ut::sw;
    constexpr ut::sw::ArgParser<Arg<std::optional<std::string>, "-i", "--in", desc<"input file">, name<"input">>> parser;
    STATIC_REQUIRE(std::get<0>(parser.args).short_flag == "-i");
    STATIC_REQUIRE(std::get<0>(parser.args).long_flag == "--in");
    STATIC_REQUIRE(std::get<0>(parser.args).name == "input");
    STATIC_REQUIRE(std::get<0>(parser.args).description == "input file");
}
