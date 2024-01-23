#include <catch.hpp>
#include <ut/pair/pair.hpp>

#include <string_view>
using namespace std::string_view_literals;
using namespace std::string_literals;

TEST_CASE("Pair construction", "[pair]") {
    SECTION("Default init") {
        ut::pair<std::string, std::vector<int>> p1;
        REQUIRE(p1.first.empty());
        REQUIRE(p1.second.empty());
        ut::pair<int, double> p2;
        REQUIRE(p2.first == 0);
        REQUIRE(p2.second == 0.0);
    }
    SECTION("Direct init") {
        ut::pair<std::string, std::string_view> p1 {"hello", "world"};
        REQUIRE(p1.first == "hello");
        REQUIRE(p1.second == "world");
        ut::pair p2 {'x', "yz"};
        REQUIRE(p2.first == 'x');
        REQUIRE(p2.second == "yz"sv);
        STATIC_REQUIRE(std::is_same_v<decltype(p2.first), char>);
        STATIC_REQUIRE(std::is_same_v<decltype(p2.second), char const *>);
        STATIC_REQUIRE(std::is_trivially_constructible_v<decltype(p2), char, char const *>);
    }
}

TEST_CASE("Pair From pairlike", "[pair]") {
    SECTION("from std::pair") {
        auto p = ut::fromPairLike(std::pair {1, 2u});
        REQUIRE(p.first == 1);
        REQUIRE(p.second == 2u);
        STATIC_REQUIRE(std::is_same_v<decltype(p), ut::pair<int, unsigned>>);
    }

    SECTION("from std::tuple") {
        auto p = ut::fromPairLike(std::tuple {1., 2.f});
        REQUIRE(p.first == 1.);
        REQUIRE(p.second == 2.f);
        STATIC_REQUIRE(std::is_same_v<decltype(p), ut::pair<double, float>>);
    }
    SECTION("from std::array") {
        auto p = ut::fromPairLike(std::array {1ll, 2ll});
        REQUIRE(p.first == 1ll);
        REQUIRE(p.second == 2ll);
        STATIC_REQUIRE(std::is_same_v<decltype(p), ut::pair<long long, long long>>);
    }
    SECTION("from ut::pair") {
        auto p = ut::fromPairLike(ut::pair {'1', 2ull});
        REQUIRE(p.first == '1');
        REQUIRE(p.second == 2ull);
        STATIC_REQUIRE(std::is_same_v<decltype(p), ut::pair<char, unsigned long long>>);
    }
}

TEST_CASE("Pair to", "[pair]") {
    SECTION("std::pair") {
        ut::pair const p {"hello"s, "world"sv};
        std::pair<std::string, std::string_view> sp = p.to<std::pair>();
        REQUIRE(sp.first == "hello");
        REQUIRE(sp.second == "world");
    }
    SECTION("std::tuple") {
        ut::pair const p {"hello"s, "world"sv};
        std::tuple<std::string, std::string_view> t = p.to<std::tuple>();
        REQUIRE(std::get<0>(t) == "hello");
        REQUIRE(std::get<1>(t) == "world");
    }
    SECTION("std::array") {
        ut::pair const p {"hello"s, "world"s};
        std::array<std::string, 2> a = p.to<std::array>();
        REQUIRE(a[0] == "hello");
        REQUIRE(a[1] == "world");
    }
    SECTION("Exact type") {
        ut::pair<char const *, char const *> const p {"hello", "world"};
        auto sp = p.to<std::pair<std::string_view, std::string>>();
        REQUIRE(sp.first == "hello");
        REQUIRE(sp.second == "world");
        STATIC_REQUIRE(std::is_same_v<decltype(sp), std::pair<std::string_view, std::string>>);
    }
}

TEST_CASE("Pair moveTo", "[pair]") {
    using StrPtr = std::unique_ptr<std::string>;
    using StrVPtr = std::unique_ptr<std::string_view>;
    SECTION("std::pair") {
        ut::pair p {std::make_unique<std::string>("hello"), std::make_unique<std::string_view>("world")};
        std::pair<StrPtr, StrVPtr> sp = p.moveTo<std::pair>();
        REQUIRE(*sp.first.get() == "hello");
        REQUIRE(*sp.second.get() == "world");
    }
    SECTION("std::tuple") {
        ut::pair p {std::make_unique<std::string>("hello"), std::make_unique<std::string_view>("world")};
        std::tuple<StrPtr, StrVPtr> t = p.moveTo<std::tuple>();
        REQUIRE(*std::get<0>(t).get() == "hello");
        REQUIRE(*std::get<1>(t).get() == "world");
    }
    SECTION("std::array") {
        ut::pair p {std::make_unique<std::string>("hello"), std::make_unique<std::string>("world")};
        std::array<StrPtr, 2> a = p.moveTo<std::array>();
        REQUIRE(*a[0].get() == "hello");
        REQUIRE(*a[1].get() == "world");
    }
    SECTION("Exact type") {
        ut::pair p {std::make_unique<int>(1), std::make_unique<double>(2.3)};
        auto sp = p.moveTo<std::pair<std::unique_ptr<int>, std::unique_ptr<double>>>();
        REQUIRE(*sp.first.get() == 1);
        REQUIRE(*sp.second.get() == 2.3);
        STATIC_REQUIRE(std::is_same_v<decltype(sp), std::pair<std::unique_ptr<int>, std::unique_ptr<double>>>);
    }
}

TEST_CASE("Pair operator==", "[pair]") {
    SECTION("std::pair") {
        const std::pair other {"123"sv, 123};
        ut::pair up1 {"123"sv, 123};
        const ut::pair up2 {"124"sv, 123};
        ut::pair up3 {"123"sv, 124};
        REQUIRE(other == up1);
        REQUIRE(up1 == other);
        REQUIRE(other != up2);
        REQUIRE(up2 != other);
        REQUIRE(other != up3);
        REQUIRE(up3 != other);
    }

    SECTION("std::tuple") {
        const std::tuple other {"123"sv, 123};
        ut::pair up1 {"123"sv, 123};
        const ut::pair up2 {"124"sv, 123};
        ut::pair up3 {"124"sv, 123};
        REQUIRE(other == up1);
        REQUIRE(up1 == other);
        REQUIRE(other != up2);
        REQUIRE(up2 != other);
        REQUIRE(other != up3);
        REQUIRE(up3 != other);
    }
    SECTION("ut::array") {
        const std::array other {"123"sv, "456"sv};
        ut::pair up1 {"123"sv, "456"sv};
        const ut::pair up2 {"124"sv, "456"sv};
        ut::pair up3 {"123"sv, "457"sv};
        REQUIRE(other == up1);
        REQUIRE(up1 == other);
        REQUIRE(other != up2);
        REQUIRE(up2 != other);
        REQUIRE(other != up3);
        REQUIRE(up3 != other);
    }
    SECTION("ut::pair") {
        const ut::pair other {"123"sv, 123};
        ut::pair up1 {"123"sv, 123};
        const ut::pair up2 {"124"sv, 123};
        ut::pair up3 {"124"sv, 123};
        REQUIRE(other == up1);
        REQUIRE(up1 == other);
        REQUIRE(other != up2);
        REQUIRE(up2 != other);
        REQUIRE(other != up3);
        REQUIRE(up3 != other);
    }
}

TEST_CASE("Pair iteration", "[pair]") {
    ut::pair p {"a"s, "b"s};
    std::array arr {"a"s, "b"s};
    SECTION("range-based-for loop") {
        for (std::size_t i = 0; auto const &s : p)
            REQUIRE(s == arr[i++]);
    }

    SECTION("iterator for loop") {
        SECTION("forward") {
            auto arrit = arr.begin();
            for (auto it = p.begin(); it != p.end(); ++it, ++arrit)
                REQUIRE(*it == *arrit);
            REQUIRE(arrit == arr.end());
        }


        SECTION("reverse") {
            auto arrit = arr.rbegin();
            for (auto it = p.rbegin(); it != p.rend(); ++it, ++arrit)
                REQUIRE(*it == *arrit);
            REQUIRE(arrit == arr.rend());
        }
    }
}
