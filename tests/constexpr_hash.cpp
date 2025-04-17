#include <catch.hpp>
#include <ut/constexpr_hash/constexpr_hash.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <iterator>
#include <string>

constexpr auto empty = "";
constexpr auto short_str = "hello";
constexpr auto long_str =
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Fusce pellentesque justo eu mauris gravida aliquet. Nam et velit tortor. Vestibulum sit amet commodo ante. Fusce eleifend tellus ac euismod porta. Proin at gravida tortor. Suspendisse eget urna vitae dui varius auctor et ac quam. Phasellus sodales sodales dolor, vel aliquet erat posuere hendrerit. Donec consectetur orci eget pulvinar pellentesque. Donec in bibendum orci. Phasellus congue, ligula non tincidunt malesuada, nisl ligula mauris. ";
constexpr auto ascii =
    "\a\b\t\n\v\f\r\"\\@!#$%&'()*+,-./0123456789:;<=>?[]^_ `abcdefghijklmnopqrstuvwxyz{|}~ABCDEFGHIJKLMNOPQRSTUVWXYZ";
constexpr auto ascii_lower =
    "\a\b\t\n\v\f\r\"\\@!#$%&'()*+,-./0123456789:;<=>?[]^_ `abcdefghijklmnopqrstuvwxyz{|}~abcdefghijklmnopqrstuvwxyz";

constexpr auto len(char const *str) {
    return std::char_traits<char>::length(str);
}

template<std::size_t size>
constexpr auto makeCopy(char const *str) {
    std::array<char, size + 1> arr {};
    std::char_traits<char>::copy(arr.data(), str, arr.size());
    arr[size] = 0;
    return arr;
}

TEST_CASE("Compile time hashing", "[constexpr_hash]") {
    static constexpr auto empty_2 = makeCopy<len(empty)>(empty);
    static constexpr auto short_str_2 = makeCopy<len(short_str)>(short_str);
    static constexpr auto long_str_2 = makeCopy<len(long_str)>(long_str);

    static constexpr auto empty_hash = ut::fnv_1a<std::size_t>(empty);
    static constexpr auto short_hash = ut::fnv_1a<std::size_t>(short_str);
    static constexpr auto long_hash = ut::fnv_1a<std::size_t>(long_str);

    static constexpr auto empty_2_hash = ut::fnv_1a<std::size_t>(empty_2.data());
    static constexpr auto short_2_hash = ut::fnv_1a<std::size_t>(short_str_2.data());
    static constexpr auto long_2_hash = ut::fnv_1a<std::size_t>(long_str_2.data());

    STATIC_REQUIRE(empty_hash == empty_2_hash);
    STATIC_REQUIRE(short_hash == short_2_hash);
    STATIC_REQUIRE(long_hash == long_2_hash);

    STATIC_REQUIRE(empty_hash != short_2_hash);
    STATIC_REQUIRE(empty_hash != long_2_hash);

    STATIC_REQUIRE(short_hash != empty_2_hash);
    STATIC_REQUIRE(short_hash != long_2_hash);

    STATIC_REQUIRE(long_hash != empty_2_hash);
    STATIC_REQUIRE(long_hash != short_2_hash);
}

TEST_CASE("Runtime hashing", "[constexpr_hash]") {
    std::srand(Catch::getSeed());
    // NOTE: we don't need very good randomness, just forcing the code to execute at runtime
    auto str1 = std::to_string(std::rand());
    auto str2 = std::to_string(std::rand());
    auto str3 = std::to_string(std::rand());

    auto str_cpy_1 = str1;
    auto str_cpy_2 = str2;
    auto str_cpy_3 = str3;

    auto str1_hash = ut::fnv_1a<std::size_t>(str1);
    auto str2_hash = ut::fnv_1a<std::size_t>(str2);
    auto str3_hash = ut::fnv_1a<std::size_t>(str3);

    auto str_cpy_1_hash = ut::fnv_1a<std::size_t>(str_cpy_1);
    auto str_cpy_2_hash = ut::fnv_1a<std::size_t>(str_cpy_2);
    auto str_cpy_3_hash = ut::fnv_1a<std::size_t>(str_cpy_3);

    REQUIRE(str1_hash == str_cpy_1_hash);
    REQUIRE(str2_hash == str_cpy_2_hash);
    REQUIRE(str3_hash == str_cpy_3_hash);

    REQUIRE(str1_hash != str_cpy_2_hash);
    REQUIRE(str1_hash != str_cpy_3_hash);

    REQUIRE(str2_hash != str_cpy_1_hash);
    REQUIRE(str2_hash != str_cpy_3_hash);

    REQUIRE(str3_hash != str_cpy_1_hash);
    REQUIRE(str3_hash != str_cpy_2_hash);
}

TEST_CASE("Compile time ignore case", "[constexpr_hash]") {

    static constexpr auto ascii_2 = makeCopy<len(ascii)>(ascii);
    static constexpr auto ascii_lower_2 = makeCopy<len(ascii_lower)>(ascii_lower);

    STATIC_REQUIRE(ut::fnv_1a<std::size_t>(ascii) == ut::fnv_1a<std::size_t>(ascii_2.data()));
    STATIC_REQUIRE(ut::fnv_1a<std::size_t, ut::CHIgnoreCase::No>(ascii)
                   == ut::fnv_1a<std::size_t, ut::CHIgnoreCase::No>(ascii_2.data()));
    STATIC_REQUIRE(ut::fnv_1a<std::size_t, ut::CHIgnoreCase::Yes>(ascii)
                   == ut::fnv_1a<std::size_t, ut::CHIgnoreCase::Yes>(ascii_2.data()));
    STATIC_REQUIRE(ut::fnv_1a<std::size_t, ut::CHIgnoreCase::Yes>(ascii)
                   != ut::fnv_1a<std::size_t, ut::CHIgnoreCase::No>(ascii_2.data()));

    STATIC_REQUIRE(ut::fnv_1a<std::size_t>(ascii) != ut::fnv_1a<std::size_t>(ascii_lower_2.data()));
    STATIC_REQUIRE(ut::fnv_1a<std::size_t, ut::CHIgnoreCase::No>(ascii)
                   != ut::fnv_1a<std::size_t, ut::CHIgnoreCase::No>(ascii_lower_2.data()));
    STATIC_REQUIRE(ut::fnv_1a<std::size_t, ut::CHIgnoreCase::Yes>(ascii)
                   == ut::fnv_1a<std::size_t, ut::CHIgnoreCase::Yes>(ascii_lower_2.data()));
    STATIC_REQUIRE(ut::fnv_1a<std::size_t, ut::CHIgnoreCase::Yes>(ascii)
                   == ut::fnv_1a<std::size_t, ut::CHIgnoreCase::No>(ascii_lower_2.data()));
}

TEST_CASE("Runtime ignore case", "[constexpr_hash]") {
    std::string str1, str2;
    std::generate_n(std::back_inserter(str1), 20, []() { return std::rand() % (128 - '!') + '!'; });
    std::transform(str1.begin(), str1.end(), std::back_inserter(str2), [](char ch) { return std::tolower(ch); });

    REQUIRE(ut::fnv_1a<std::size_t>(str1) != ut::fnv_1a<std::size_t>(str2));
    REQUIRE(ut::fnv_1a<std::size_t, ut::CHIgnoreCase::No>(str1) != ut::fnv_1a<std::size_t, ut::CHIgnoreCase::No>(str2));
    REQUIRE(ut::fnv_1a<std::size_t, ut::CHIgnoreCase::Yes>(str1) == ut::fnv_1a<std::size_t, ut::CHIgnoreCase::Yes>(str2));
    REQUIRE(ut::fnv_1a<std::size_t, ut::CHIgnoreCase::Yes>(str1) == ut::fnv_1a<std::size_t, ut::CHIgnoreCase::No>(str2));
}
