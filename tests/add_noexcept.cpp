#include <catch.hpp>
#include <ut/add_noexcept/add_noexcept.hpp>

void can_throw1() { }

void can_throw2(int) { }

void can_throw3(int (*)()) { }

void can_throw4(int (*)() noexcept) { }

void cant_throw1() noexcept { }

void cant_throw2(int) noexcept { }

void cant_throw3(int (*)()) noexcept { }

void cant_throw4(int (*)() noexcept) noexcept { }

TEST_CASE("Adding noexcept to function type", "[add_noexcept]") {

    SECTION("Adding noexcept to throwing function") {
        STATIC_REQUIRE(std::is_same_v<ut::add_noexcept_t<decltype(can_throw1)>, decltype(cant_throw1)>);
        STATIC_REQUIRE(std::is_same_v<ut::add_noexcept_t<decltype(can_throw2)>, decltype(cant_throw2)>);
        STATIC_REQUIRE(std::is_same_v<ut::add_noexcept_t<decltype(can_throw3)>, decltype(cant_throw3)>);
        STATIC_REQUIRE(std::is_same_v<ut::add_noexcept_t<decltype(can_throw4)>, decltype(cant_throw4)>);
    }

    SECTION("Adding noexcept to noexcept function") {
        STATIC_REQUIRE(std::is_same_v<ut::add_noexcept_t<decltype(cant_throw1)>, decltype(cant_throw1)>);
        STATIC_REQUIRE(std::is_same_v<ut::add_noexcept_t<decltype(cant_throw2)>, decltype(cant_throw2)>);
        STATIC_REQUIRE(std::is_same_v<ut::add_noexcept_t<decltype(cant_throw3)>, decltype(cant_throw3)>);
        STATIC_REQUIRE(std::is_same_v<ut::add_noexcept_t<decltype(cant_throw4)>, decltype(cant_throw4)>);
    }
}

TEST_CASE("Using the cast function", "[add_noexcept]") {

    SECTION("Casting throwing function") {
        auto cast1 = ut::noexcept_cast(can_throw1);
        auto cast2 = ut::noexcept_cast(can_throw2);
        auto cast3 = ut::noexcept_cast(can_throw3);
        auto cast4 = ut::noexcept_cast(can_throw4);
        STATIC_REQUIRE(std::is_same_v<decltype(cast1), decltype(cant_throw1) *>);
        STATIC_REQUIRE(std::is_same_v<decltype(cast2), decltype(cant_throw2) *>);
        STATIC_REQUIRE(std::is_same_v<decltype(cast3), decltype(cant_throw3) *>);
        STATIC_REQUIRE(std::is_same_v<decltype(cast4), decltype(cant_throw4) *>);
    }

    SECTION("Casting noexcept function") {
        auto cast1 = ut::noexcept_cast(cant_throw1);
        auto cast2 = ut::noexcept_cast(cant_throw2);
        auto cast3 = ut::noexcept_cast(cant_throw3);
        auto cast4 = ut::noexcept_cast(cant_throw4);
        STATIC_REQUIRE(std::is_same_v<decltype(cast1), decltype(cant_throw1) *>);
        STATIC_REQUIRE(std::is_same_v<decltype(cast2), decltype(cant_throw2) *>);
        STATIC_REQUIRE(std::is_same_v<decltype(cast3), decltype(cant_throw3) *>);
        STATIC_REQUIRE(std::is_same_v<decltype(cast4), decltype(cant_throw4) *>);
    }
}
