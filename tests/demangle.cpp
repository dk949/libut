#include <catch.hpp>
#include <ut/demangle/demangle.hpp>

// NOTE: since this function depends on compiler extensions and platform define behaviour, it's quite hard to
// test. These test just ensure it returns something remotely sensible.



namespace Lorem {
struct Ipsum {
    class Dolor { };
};
};

TEST_CASE("Demangle", "[demangle]") {
    using namespace Catch::Matchers;

    SECTION("simple") {
        auto const i1 = ut::typeName(int {});
        auto const i2 = ut::typeName<int>();

        auto const d1 = ut::typeName(double {});
        auto const d2 = ut::typeName<double>();

        REQUIRE(i1 == i2);
        REQUIRE(d1 == d2);

        REQUIRE_THAT(i1, ContainsSubstring("int"));
        REQUIRE_THAT(i2, ContainsSubstring("int"));
        REQUIRE_THAT(d1, ContainsSubstring("double"));
        REQUIRE_THAT(d2, ContainsSubstring("double"));
    }

    SECTION("complex") {
        auto const ipsum1 = ut::typeName(Lorem::Ipsum {});
        auto const ipsum2 = ut::typeName<Lorem::Ipsum>();

        auto const dolor1 = ut::typeName(Lorem::Ipsum::Dolor {});
        auto const dolor2 = ut::typeName<Lorem::Ipsum::Dolor>();

        REQUIRE(ipsum1 == ipsum2);
        REQUIRE(dolor1 == dolor2);
        REQUIRE_THAT(ipsum1, ContainsSubstring("Lorem") && ContainsSubstring("Ipsum"));
        REQUIRE_THAT(ipsum2, ContainsSubstring("Lorem") && ContainsSubstring("Ipsum"));
        REQUIRE_THAT(dolor1, ContainsSubstring("Lorem") && ContainsSubstring("Ipsum") && ContainsSubstring("Dolor"));
        REQUIRE_THAT(dolor2, ContainsSubstring("Lorem") && ContainsSubstring("Ipsum") && ContainsSubstring("Dolor"));
    }
}
