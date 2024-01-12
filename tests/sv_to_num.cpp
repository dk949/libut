#include <catch.hpp>
#include <ut/sv_to_num/sv_to_num.hpp>

#include <string_view>
using namespace std::string_view_literals;
using namespace ut;

TEST_CASE("Integer tests", "[sv_to_num]") {
    auto const base_10_1234 = "1234"sv;
    auto const base_10_8589934592 = "8589934592"sv;
    auto const base_10_negative_1234 = "-1234"sv;
    auto const base_16_234e8 = "234e8"sv;
    auto const partially_incorrect = "1234%!"sv;
    auto const fully_incorrect = "..xx"sv;
    auto const empty = ""sv;
    auto const null = std::string_view {};

    SECTION("full") {
        REQUIRE(svToInt(base_10_1234).has_value());
        REQUIRE(svToInt(base_10_1234).value() == 1234);
        REQUIRE_FALSE(svToInt<int>(base_10_8589934592).has_value());
        REQUIRE(svToInt<long long>(base_10_8589934592).has_value());
        REQUIRE(svToInt<long long>(base_10_8589934592).value() == 8'589'934'592l);
        REQUIRE_FALSE(svToInt(base_16_234e8).has_value());
        REQUIRE(svToInt<long long, SvToNumBase(16)>(base_16_234e8).has_value());
        REQUIRE(svToInt<long long, SvToNumBase(16)>(base_16_234e8).value() == 0x234e8);
        REQUIRE(svToInt(base_10_negative_1234).has_value());
        REQUIRE(svToInt(base_10_negative_1234).value() == -1234);
        REQUIRE_FALSE(svToInt<unsigned>(base_10_negative_1234).has_value());
        REQUIRE_FALSE(svToInt(partially_incorrect).has_value());
        REQUIRE_FALSE(svToInt(fully_incorrect).has_value());
        REQUIRE_FALSE(svToInt(empty).has_value());
        REQUIRE_FALSE(svToInt(null).has_value());
    }
    SECTION("partial") {
        REQUIRE(svToInt<long long, SvToNumBase(10), SvToNumOnlyNumbers::No>(base_10_1234).has_value());
        REQUIRE(svToInt<long long, SvToNumBase(10), SvToNumOnlyNumbers::No>(base_10_1234).value().first == 1234);
        REQUIRE(svToInt<long long, SvToNumBase(10), SvToNumOnlyNumbers::No>(base_10_1234).value().second == "");
        REQUIRE(svToInt<long long, SvToNumBase(10), SvToNumOnlyNumbers::No>(partially_incorrect).has_value());
        REQUIRE(svToInt<long long, SvToNumBase(10), SvToNumOnlyNumbers::No>(partially_incorrect).value().first == 1234);
        REQUIRE(svToInt<long long, SvToNumBase(10), SvToNumOnlyNumbers::No>(partially_incorrect).value().second == "%!");
        REQUIRE_FALSE(svToInt<long long, SvToNumBase(10), SvToNumOnlyNumbers::No>(fully_incorrect).has_value());
        REQUIRE_FALSE(svToInt<long long, SvToNumBase(10), SvToNumOnlyNumbers::No>(empty).has_value());
        REQUIRE_FALSE(svToInt<long long, SvToNumBase(10), SvToNumOnlyNumbers::No>(null).has_value());
    }
}

TEST_CASE("Floating point tests", "[sv_to_num]") {
    auto const base_10_1234 = "1234"sv;
    auto const base_10_12_34 = "12.34"sv;
    auto const base_10_1234_ = "1234."sv;
    auto const base_10__1234 = ".1234"sv;
    auto const base_10_negative_12_34 = "-12.34"sv;
    auto const base_10_negative_1234_ = "-1234."sv;
    auto const base_10_negative__1234 = "-.1234"sv;
    auto const base_16_abcd = "abcd";
    auto const base_16_ab_cd = "ab.cd";
    auto const base_10_0_5857438957348975034827504780 = "0.5857438957348975034827504780";
    auto const partially_incorrect = "1.2x"sv;
    auto const fully_incorrect = "--)"sv;
    auto const empty = ""sv;
    auto const null = std::string_view {};

    SECTION("full") {
        REQUIRE(svToFloat(base_10_1234).has_value());
        REQUIRE_THAT(svToFloat(base_10_1234).value(), Catch::Matchers::WithinRel(1234.0));
        REQUIRE(svToFloat(base_10_12_34).has_value());
        REQUIRE_THAT(svToFloat(base_10_12_34).value(), Catch::Matchers::WithinRel(12.34));
        REQUIRE(svToFloat(base_10_1234_).has_value());
        REQUIRE_THAT(svToFloat(base_10_1234_).value(), Catch::Matchers::WithinRel(1234.));
        REQUIRE(svToFloat(base_10__1234).has_value());
        REQUIRE_THAT(svToFloat(base_10__1234).value(), Catch::Matchers::WithinRel(.1234));
        REQUIRE(svToFloat(base_10_negative_12_34).has_value());
        REQUIRE_THAT(svToFloat(base_10_negative_12_34).value(), Catch::Matchers::WithinRel(-12.34));
        REQUIRE(svToFloat(base_10_negative_1234_).has_value());
        REQUIRE_THAT(svToFloat(base_10_negative_1234_).value(), Catch::Matchers::WithinRel(-1234.));
        REQUIRE(svToFloat(base_10_negative__1234).has_value());
        REQUIRE_THAT(svToFloat(base_10_negative__1234).value(), Catch::Matchers::WithinRel(-.1234));
        REQUIRE_FALSE(svToFloat(base_16_abcd).has_value());
        REQUIRE(svToFloat<double, SvToNumFormat::Hex>(base_16_abcd).has_value());
        REQUIRE_THAT((svToFloat<double, SvToNumFormat::Hex>(base_16_abcd).value()),
            Catch::Matchers::WithinRel(double(0xabcd)));
        REQUIRE_FALSE(svToFloat(base_16_ab_cd).has_value());
        REQUIRE(svToFloat<double, SvToNumFormat::Hex>(base_16_ab_cd).has_value());
        REQUIRE_THAT((svToFloat<double, SvToNumFormat::Hex>(base_16_ab_cd).value()),
            Catch::Matchers::WithinRel(0xab.cdp0));
        REQUIRE(svToFloat(base_10_0_5857438957348975034827504780).has_value());
        REQUIRE_THAT(svToFloat(base_10_0_5857438957348975034827504780).value(),
            Catch::Matchers::WithinRel(0.5857438957348975034827504780));
        REQUIRE(svToFloat<float>(base_10_0_5857438957348975034827504780).has_value());
        REQUIRE_THAT(svToFloat<float>(base_10_0_5857438957348975034827504780).value(),
            Catch::Matchers::WithinRel(0.5857438957348975034827504780f));
        REQUIRE_FALSE(svToFloat(partially_incorrect).has_value());
        REQUIRE_FALSE(svToFloat(fully_incorrect).has_value());
        REQUIRE_FALSE(svToFloat(empty).has_value());
        REQUIRE_FALSE(svToFloat(null).has_value());
    }
    SECTION("partial") {
        REQUIRE(svToFloat<double, SvToNumFormat::Default, SvToNumOnlyNumbers::No>(base_10_12_34).has_value());
        REQUIRE_THAT((svToFloat<double, SvToNumFormat::Default, SvToNumOnlyNumbers::No>(base_10_12_34).value()).first,
            Catch::Matchers::WithinRel(12.34));
        REQUIRE(svToFloat<double, SvToNumFormat::Default, SvToNumOnlyNumbers::No>(base_10_12_34).value().second == "");
        REQUIRE(svToFloat<double, SvToNumFormat::Default, SvToNumOnlyNumbers::No>(partially_incorrect).has_value());
        REQUIRE_THAT((svToFloat<double, SvToNumFormat::Default, SvToNumOnlyNumbers::No>(partially_incorrect).value().first),
            Catch::Matchers::WithinRel(1.2));
        REQUIRE(svToFloat<double, SvToNumFormat::Default, SvToNumOnlyNumbers::No>(partially_incorrect).value().second
                == "x");
        REQUIRE_FALSE(svToFloat<double, SvToNumFormat::Default, SvToNumOnlyNumbers::No>(fully_incorrect).has_value());
        REQUIRE_FALSE(svToFloat<double, SvToNumFormat::Default, SvToNumOnlyNumbers::No>(empty).has_value());
        REQUIRE_FALSE(svToFloat<double, SvToNumFormat::Default, SvToNumOnlyNumbers::No>(null).has_value());
    }
}
