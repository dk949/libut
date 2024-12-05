#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include <sstream>
#if __clang_major__ >= 19 || __GNUC__ >= 14  // some features not supported on earlier compilers

#    include <catch.hpp>
#    include <ut/static_string/static_string.hpp>

using ut::BasicStaticString;
using ut::StaticString;
using ut::U16StaticString;
using ut::U32StaticString;
using ut::U8StaticString;
using ut::WStaticString;

using namespace std::string_view_literals;
using namespace std::string_view_literals;


// clang-format off
template<typename T> struct StringGetter;
template<> struct StringGetter<char>     { static constexpr auto str = "hello"sv;};
template<> struct StringGetter<char8_t>  { static constexpr auto str = u8"hello"sv; };
template<> struct StringGetter<char16_t> { static constexpr auto str = u"hello"sv; };
template<> struct StringGetter<char32_t> { static constexpr auto str = U"hello"sv; };
template<> struct StringGetter<wchar_t>  { static constexpr auto str = L"hello"sv; };

template<typename T> struct EmptyStringGetter;
template<> struct EmptyStringGetter<char>     { static constexpr auto str = ""sv;};
template<> struct EmptyStringGetter<char8_t>  { static constexpr auto str = u8""sv; };
template<> struct EmptyStringGetter<char16_t> { static constexpr auto str = u""sv; };
template<> struct EmptyStringGetter<char32_t> { static constexpr auto str = U""sv; };
template<> struct EmptyStringGetter<wchar_t>  { static constexpr auto str = L""sv; };

// clang-format on

// TEMPLATE_TEST_CASE("static_string constructors", "[static_string]", char, wchar_t, char8_t, char16_t, char32_t){
TEST_CASE("static_string constructors", "[static_string]") {
    SECTION("from char full deduction") {
        constexpr BasicStaticString s = "hello";
        constexpr BasicStaticString w = L"hello";
        constexpr BasicStaticString u8 = u8"hello";
        constexpr BasicStaticString u16 = u"hello";
        constexpr BasicStaticString u32 = U"hello";
        STATIC_REQUIRE(s.size() == 5);
        STATIC_REQUIRE(w.size() == 5);
        STATIC_REQUIRE(u8.size() == 5);
        STATIC_REQUIRE(u16.size() == 5);
        STATIC_REQUIRE(u32.size() == 5);
    }
    SECTION("from char aliases deduction") {
        constexpr StaticString s = "hello";
        constexpr WStaticString w = L"hello";
        constexpr U8StaticString u8 = u8"hello";
        constexpr U16StaticString u16 = u"hello";
        constexpr U32StaticString u32 = U"hello";
        STATIC_REQUIRE(s.size() == 5);
        STATIC_REQUIRE(w.size() == 5);
        STATIC_REQUIRE(u8.size() == 5);
        STATIC_REQUIRE(u16.size() == 5);
        STATIC_REQUIRE(u32.size() == 5);
    }
    SECTION("from string_view") {
        constexpr StaticString<10> s {"hello"sv};
        constexpr WStaticString<10> w {L"hello"sv};
        constexpr U8StaticString<10> u8 {u8"hello"sv};
        constexpr U16StaticString<10> u16 {u"hello"sv};
        constexpr U32StaticString<10> u32 {U"hello"sv};
        STATIC_REQUIRE(s.size() == 5);
        STATIC_REQUIRE(w.size() == 5);
        STATIC_REQUIRE(u8.size() == 5);
        STATIC_REQUIRE(u16.size() == 5);
        STATIC_REQUIRE(u32.size() == 5);
    }
}

TEMPLATE_TEST_CASE("static_string iterators", "[static_string]", char, char8_t, char16_t, char32_t, wchar_t) {
    static constexpr BasicStaticString<TestType, 5> str {StringGetter<TestType>::str};
    constexpr int res = []() {
        int acc = 0;
        for (auto ch : str) {
            acc += int(ch);
        }
        return acc;
    }();
    STATIC_REQUIRE(res == 532);
    STATIC_REQUIRE(*str.begin() == 'h');
    STATIC_REQUIRE(*str.cbegin() == 'h');
    STATIC_REQUIRE(*str.rbegin() == 'o');
    STATIC_REQUIRE(*str.crbegin() == 'o');

    STATIC_REQUIRE(*std::prev(str.end()) == 'o');
    STATIC_REQUIRE(*std::prev(str.cend()) == 'o');
    STATIC_REQUIRE(*std::prev(str.rend()) == 'h');
    STATIC_REQUIRE(*std::prev(str.crend()) == 'h');
}

TEMPLATE_TEST_CASE("static_string isNullTerminated", "[static_string]", char, char8_t, char16_t, char32_t, wchar_t) {
    static constexpr BasicStaticString<TestType, 5> str1 {StringGetter<TestType>::str};
    static constexpr BasicStaticString<TestType, 6> str2 {StringGetter<TestType>::str};
    static constexpr BasicStaticString<TestType, 20> str3 {StringGetter<TestType>::str};
    STATIC_REQUIRE(!str1.isNullTerminated());
    STATIC_REQUIRE(str2.isNullTerminated());
    STATIC_REQUIRE(str3.isNullTerminated());
    STATIC_REQUIRE(*str2.end() == 0);
    STATIC_REQUIRE(*str3.end() == 0);
}

TEMPLATE_TEST_CASE("static_string accessors", "[static_string]", char, char8_t, char16_t, char32_t, wchar_t) {
    static constexpr BasicStaticString<TestType, 20> str {StringGetter<TestType>::str};
    STATIC_REQUIRE(str[0] == 'h');
    STATIC_REQUIRE(str.front() == 'h');
    STATIC_REQUIRE(str.back() == 'o');
    STATIC_REQUIRE(*str.data() == 'h');
    REQUIRE_THROWS(str.at(5));
}

TEMPLATE_TEST_CASE("static_string capacity", "[static_string]", char, char8_t, char16_t, char32_t, wchar_t) {
    static constexpr BasicStaticString<TestType, 20> str {StringGetter<TestType>::str};
    static constexpr BasicStaticString<TestType, 20> empty {EmptyStringGetter<TestType>::str};
    static constexpr BasicStaticString<TestType, 1> really_empty {EmptyStringGetter<TestType>::str};
    SECTION("non-empty") {
        STATIC_REQUIRE(str.size() == 5);
        STATIC_REQUIRE(str.length() == 5);
        STATIC_REQUIRE(str.capacity() == 20);
        STATIC_REQUIRE(str.max_size() == 20);
        STATIC_REQUIRE(str.empty() == false);
    }

    SECTION("empty") {
        STATIC_REQUIRE(empty.size() == 0);
        STATIC_REQUIRE(empty.length() == 0);
        STATIC_REQUIRE(empty.capacity() == 20);
        STATIC_REQUIRE(empty.max_size() == 20);
        STATIC_REQUIRE(empty.empty() == true);
    }

    SECTION("very empry") {
        STATIC_REQUIRE(really_empty.size() == 0);
        STATIC_REQUIRE(really_empty.length() == 0);
        STATIC_REQUIRE(really_empty.capacity() == 1);
        STATIC_REQUIRE(really_empty.max_size() == 1);
        STATIC_REQUIRE(really_empty.empty() == true);
    }
}

TEMPLATE_TEST_CASE("static_string swap", "[static_string]", char, char8_t, char16_t, char32_t, wchar_t) {
    SECTION("member swap") {
        static constexpr auto str_pair = []() {
            BasicStaticString<TestType, 20> s {StringGetter<TestType>::str};
            BasicStaticString<TestType, 20> s2 {EmptyStringGetter<TestType>::str};
            s.swap(s2);
            return std::pair {s, s2};
        }();
        STATIC_REQUIRE(str_pair.first.empty());
        STATIC_REQUIRE(str_pair.second.view() == StringGetter<TestType>::str);
    }
    SECTION("non-member swap") {
        static constexpr auto str_pair = []() {
            BasicStaticString<TestType, 20> s {StringGetter<TestType>::str};
            BasicStaticString<TestType, 20> s2 {EmptyStringGetter<TestType>::str};
            swap(s, s2);
            return std::pair {s, s2};
        }();
        STATIC_REQUIRE(str_pair.first.empty());
        STATIC_REQUIRE(str_pair.second.view() == StringGetter<TestType>::str);
    }
}

TEMPLATE_TEST_CASE("static_string leftshift", "[static_string]", char, char8_t, char16_t, char32_t, wchar_t) {
    std::basic_stringstream<TestType> ss;
    static constexpr BasicStaticString<TestType, 20> str {StringGetter<TestType>::str};
    ss << str;
    auto s = ss.str();
    std::basic_string_view<TestType> sv {s};
    REQUIRE(sv == str.view());
}

#endif
