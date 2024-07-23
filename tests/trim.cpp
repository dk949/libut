#include <catch.hpp>
#include <ut/trim/trim.hpp>

#include <string_view>
using namespace std::string_view_literals;


auto const empty = ""sv;
auto const none = "none"sv;
auto const only = "\t   \n"sv;
auto const left = "\t   \nleft"sv;
auto const right = "right\t   \n"sv;
auto const both = "\t   \nboth\t   \n"sv;

auto const num_only = "12345"sv;
auto const num_left = "12345left"sv;
auto const num_right = "right12345"sv;
auto const num_both = "12345both12345"sv;


auto const pred = [](char ch) {
    return !std::isdigit(ch);
};

TEST_CASE("Trim left", "[trim]") {
    REQUIRE(ut::trim_left(empty) == empty);
    REQUIRE(ut::trim_left(none) == none);
    REQUIRE(ut::trim_left(only) == ""sv);
    REQUIRE(ut::trim_left(left) == "left"sv);
    REQUIRE(ut::trim_left(right) == right);
    REQUIRE(ut::trim_left(both) == "both\t   \n"sv);
}

TEST_CASE("Trim right", "[trim]") {
    REQUIRE(ut::trim_right(empty) == empty);
    REQUIRE(ut::trim_right(none) == none);
    REQUIRE(ut::trim_right(only) == ""sv);
    REQUIRE(ut::trim_right(left) == left);
    REQUIRE(ut::trim_right(right) == "right"sv);
    REQUIRE(ut::trim_right(both) == "\t   \nboth"sv);
}

TEST_CASE("Trim both", "[trim]") {
    REQUIRE(ut::trim(empty) == empty);
    REQUIRE(ut::trim(none) == none);
    REQUIRE(ut::trim(only) == ""sv);
    REQUIRE(ut::trim(left) == "left"sv);
    REQUIRE(ut::trim(right) == "right"sv);
    REQUIRE(ut::trim(both) == "both"sv);
}

TEST_CASE("Trim left custom predicate", "[trim]") {
    REQUIRE(ut::trim_left(empty, pred) == empty);
    REQUIRE(ut::trim_left(none, pred) == none);
    REQUIRE(ut::trim_left(num_only, pred) == ""sv);
    REQUIRE(ut::trim_left(num_left, pred) == "left"sv);
    REQUIRE(ut::trim_left(num_right, pred) == num_right);
    REQUIRE(ut::trim_left(num_both, pred) == "both12345"sv);
}

TEST_CASE("Trim right custom predicate", "[trim]") {
    REQUIRE(ut::trim_right(empty, pred) == empty);
    REQUIRE(ut::trim_right(none, pred) == none);
    REQUIRE(ut::trim_right(num_only, pred) == ""sv);
    REQUIRE(ut::trim_right(num_left, pred) == num_left);
    REQUIRE(ut::trim_right(num_right, pred) == "right"sv);
    REQUIRE(ut::trim_right(num_both, pred) == "12345both"sv);
}

TEST_CASE("Trim both custom predicate", "[trim]") {
    REQUIRE(ut::trim(empty, pred) == empty);
    REQUIRE(ut::trim(none, pred) == none);
    REQUIRE(ut::trim(num_only, pred) == ""sv);
    REQUIRE(ut::trim(num_left, pred) == "left"sv);
    REQUIRE(ut::trim(num_right, pred) == "right"sv);
    REQUIRE(ut::trim(num_both, pred) == "both"sv);
}
