#include <catch.hpp>
#include <catch2/catch_test_macros.hpp>
#include <ut/copy_traits/copy_traits.hpp>

TEST_CASE("Copy const to another type", "[copy_traits]") {

    using T = int;

    using U = struct { };

    STATIC_REQUIRE(std::same_as<ut::copy_const_t<U, T>, T>);
    STATIC_REQUIRE(std::same_as<ut::copy_const_t<U, T &>, T &>);
    STATIC_REQUIRE(std::same_as<ut::copy_const_t<U, T const>, T>);
    STATIC_REQUIRE(std::same_as<ut::copy_const_t<U, T const &>, T &>);

    STATIC_REQUIRE(std::same_as<ut::copy_const_t<U &, T>, T>);
    STATIC_REQUIRE(std::same_as<ut::copy_const_t<U &, T &>, T &>);
    STATIC_REQUIRE(std::same_as<ut::copy_const_t<U &, T const>, T>);
    STATIC_REQUIRE(std::same_as<ut::copy_const_t<U &, T const &>, T &>);

    STATIC_REQUIRE(std::same_as<ut::copy_const_t<U const, T>, T const>);
    STATIC_REQUIRE(std::same_as<ut::copy_const_t<U const, T &>, T const &>);
    STATIC_REQUIRE(std::same_as<ut::copy_const_t<U const, T const>, T const>);
    STATIC_REQUIRE(std::same_as<ut::copy_const_t<U const, T const &>, T const &>);

    STATIC_REQUIRE(std::same_as<ut::copy_const_t<U const &, T>, T const>);
    STATIC_REQUIRE(std::same_as<ut::copy_const_t<U const &, T &>, T const &>);
    STATIC_REQUIRE(std::same_as<ut::copy_const_t<U const &, T const>, T const>);
    STATIC_REQUIRE(std::same_as<ut::copy_const_t<U const &, T const &>, T const &>);

    STATIC_REQUIRE(std::same_as<ut::copy_const_t<U, T>, T>);
    STATIC_REQUIRE(std::same_as<ut::copy_const_t<U, T &&>, T &&>);
    STATIC_REQUIRE(std::same_as<ut::copy_const_t<U, T const>, T>);
    STATIC_REQUIRE(std::same_as<ut::copy_const_t<U, T const &&>, T &&>);

    STATIC_REQUIRE(std::same_as<ut::copy_const_t<U &&, T>, T>);
    STATIC_REQUIRE(std::same_as<ut::copy_const_t<U &&, T &&>, T &&>);
    STATIC_REQUIRE(std::same_as<ut::copy_const_t<U &&, T const>, T>);
    STATIC_REQUIRE(std::same_as<ut::copy_const_t<U &&, T const &&>, T &&>);

    STATIC_REQUIRE(std::same_as<ut::copy_const_t<U const, T>, T const>);
    STATIC_REQUIRE(std::same_as<ut::copy_const_t<U const, T &&>, T const &&>);
    STATIC_REQUIRE(std::same_as<ut::copy_const_t<U const, T const>, T const>);
    STATIC_REQUIRE(std::same_as<ut::copy_const_t<U const, T const &&>, T const &&>);

    STATIC_REQUIRE(std::same_as<ut::copy_const_t<U const &&, T>, T const>);
    STATIC_REQUIRE(std::same_as<ut::copy_const_t<U const &&, T &&>, T const &&>);
    STATIC_REQUIRE(std::same_as<ut::copy_const_t<U const &&, T const>, T const>);
    STATIC_REQUIRE(std::same_as<ut::copy_const_t<U const &&, T const &&>, T const &&>);
}

TEST_CASE("Copy reference to another type", "[copy_traits]") {

    using T = int;

    using U = struct { };

    STATIC_REQUIRE(std::same_as<ut::copy_reference_t<U, T>, T>);
    STATIC_REQUIRE(std::same_as<ut::copy_reference_t<U, T &>, T>);
    STATIC_REQUIRE(std::same_as<ut::copy_reference_t<U, T const>, T const>);
    STATIC_REQUIRE(std::same_as<ut::copy_reference_t<U, T const &>, T const>);

    STATIC_REQUIRE(std::same_as<ut::copy_reference_t<U &, T>, T &>);
    STATIC_REQUIRE(std::same_as<ut::copy_reference_t<U &, T &>, T &>);
    STATIC_REQUIRE(std::same_as<ut::copy_reference_t<U &, T const>, T const &>);
    STATIC_REQUIRE(std::same_as<ut::copy_reference_t<U &, T const &>, T const &>);

    STATIC_REQUIRE(std::same_as<ut::copy_reference_t<U const, T>, T>);
    STATIC_REQUIRE(std::same_as<ut::copy_reference_t<U const, T &>, T>);
    STATIC_REQUIRE(std::same_as<ut::copy_reference_t<U const, T const>, T const>);
    STATIC_REQUIRE(std::same_as<ut::copy_reference_t<U const, T const &>, T const>);

    STATIC_REQUIRE(std::same_as<ut::copy_reference_t<U const &, T>, T &>);
    STATIC_REQUIRE(std::same_as<ut::copy_reference_t<U const &, T &>, T &>);
    STATIC_REQUIRE(std::same_as<ut::copy_reference_t<U const &, T const>, T const &>);
    STATIC_REQUIRE(std::same_as<ut::copy_reference_t<U const &, T const &>, T const &>);

    STATIC_REQUIRE(std::same_as<ut::copy_reference_t<U, T>, T>);
    STATIC_REQUIRE(std::same_as<ut::copy_reference_t<U, T &&>, T>);
    STATIC_REQUIRE(std::same_as<ut::copy_reference_t<U, T const>, T const>);
    STATIC_REQUIRE(std::same_as<ut::copy_reference_t<U, T const &&>, T const>);

    STATIC_REQUIRE(std::same_as<ut::copy_reference_t<U &&, T>, T &&>);
    STATIC_REQUIRE(std::same_as<ut::copy_reference_t<U &&, T &&>, T &&>);
    STATIC_REQUIRE(std::same_as<ut::copy_reference_t<U &&, T const>, T const &&>);
    STATIC_REQUIRE(std::same_as<ut::copy_reference_t<U &&, T const &&>, T const &&>);

    STATIC_REQUIRE(std::same_as<ut::copy_reference_t<U const, T>, T>);
    STATIC_REQUIRE(std::same_as<ut::copy_reference_t<U const, T &&>, T>);
    STATIC_REQUIRE(std::same_as<ut::copy_reference_t<U const, T const>, T const>);
    STATIC_REQUIRE(std::same_as<ut::copy_reference_t<U const, T const &&>, T const>);

    STATIC_REQUIRE(std::same_as<ut::copy_reference_t<U const &&, T>, T &&>);
    STATIC_REQUIRE(std::same_as<ut::copy_reference_t<U const &&, T &&>, T &&>);
    STATIC_REQUIRE(std::same_as<ut::copy_reference_t<U const &&, T const>, T const &&>);
    STATIC_REQUIRE(std::same_as<ut::copy_reference_t<U const &&, T const &&>, T const &&>);
}

//
TEST_CASE("Copy volatile to another type", "[copy_traits]") {

    using T = int;

    using U = struct { };

    STATIC_REQUIRE(std::same_as<ut::copy_volatile_t<U, T>, T>);
    STATIC_REQUIRE(std::same_as<ut::copy_volatile_t<U, T &>, T &>);
    STATIC_REQUIRE(std::same_as<ut::copy_volatile_t<U, volatile T>, T>);
    STATIC_REQUIRE(std::same_as<ut::copy_volatile_t<U, volatile T &>, T &>);

    STATIC_REQUIRE(std::same_as<ut::copy_volatile_t<U &, T>, T>);
    STATIC_REQUIRE(std::same_as<ut::copy_volatile_t<U &, T &>, T &>);
    STATIC_REQUIRE(std::same_as<ut::copy_volatile_t<U &, volatile T>, T>);
    STATIC_REQUIRE(std::same_as<ut::copy_volatile_t<U &, volatile T &>, T &>);

    STATIC_REQUIRE(std::same_as<ut::copy_volatile_t<volatile U, T>, volatile T>);
    STATIC_REQUIRE(std::same_as<ut::copy_volatile_t<volatile U, T &>, volatile T &>);
    STATIC_REQUIRE(std::same_as<ut::copy_volatile_t<volatile U, volatile T>, volatile T>);
    STATIC_REQUIRE(std::same_as<ut::copy_volatile_t<volatile U, volatile T &>, volatile T &>);

    STATIC_REQUIRE(std::same_as<ut::copy_volatile_t<volatile U &, T>, volatile T>);
    STATIC_REQUIRE(std::same_as<ut::copy_volatile_t<volatile U &, T &>, volatile T &>);
    STATIC_REQUIRE(std::same_as<ut::copy_volatile_t<volatile U &, volatile T>, volatile T>);
    STATIC_REQUIRE(std::same_as<ut::copy_volatile_t<volatile U &, volatile T &>, volatile T &>);

    STATIC_REQUIRE(std::same_as<ut::copy_volatile_t<U, T>, T>);
    STATIC_REQUIRE(std::same_as<ut::copy_volatile_t<U, T &&>, T &&>);
    STATIC_REQUIRE(std::same_as<ut::copy_volatile_t<U, volatile T>, T>);
    STATIC_REQUIRE(std::same_as<ut::copy_volatile_t<U, volatile T &&>, T &&>);

    STATIC_REQUIRE(std::same_as<ut::copy_volatile_t<U &&, T>, T>);
    STATIC_REQUIRE(std::same_as<ut::copy_volatile_t<U &&, T &&>, T &&>);
    STATIC_REQUIRE(std::same_as<ut::copy_volatile_t<U &&, volatile T>, T>);
    STATIC_REQUIRE(std::same_as<ut::copy_volatile_t<U &&, volatile T &&>, T &&>);

    STATIC_REQUIRE(std::same_as<ut::copy_volatile_t<volatile U, T>, volatile T>);
    STATIC_REQUIRE(std::same_as<ut::copy_volatile_t<volatile U, T &&>, volatile T &&>);
    STATIC_REQUIRE(std::same_as<ut::copy_volatile_t<volatile U, volatile T>, volatile T>);
    STATIC_REQUIRE(std::same_as<ut::copy_volatile_t<volatile U, volatile T &&>, volatile T &&>);

    STATIC_REQUIRE(std::same_as<ut::copy_volatile_t<volatile U &&, T>, volatile T>);
    STATIC_REQUIRE(std::same_as<ut::copy_volatile_t<volatile U &&, T &&>, volatile T &&>);
    STATIC_REQUIRE(std::same_as<ut::copy_volatile_t<volatile U &&, volatile T>, volatile T>);
    STATIC_REQUIRE(std::same_as<ut::copy_volatile_t<volatile U &&, volatile T &&>, volatile T &&>);
}

TEST_CASE("Copy cvref to another type", "[copy_traits]") {

    using T = int;

    using U = struct { };

    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U, T>, T>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U, T &>, T>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U, T const>, T>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U, T const &>, T>);

    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U &, T>, T &>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U &, T &>, T &>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U &, T const>, T &>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U &, T const &>, T &>);

    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U const, T>, T const>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U const, T &>, T const>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U const, T const>, T const>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U const, T const &>, T const>);

    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U const &, T>, T const &>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U const &, T &>, T const &>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U const &, T const>, T const &>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U const &, T const &>, T const &>);

    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U, T>, T>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U, T &&>, T>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U, T const>, T>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U, T const &&>, T>);

    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U &&, T>, T &&>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U &&, T &&>, T &&>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U &&, T const>, T &&>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U &&, T const &&>, T &&>);

    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U const, T>, T const>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U const, T &&>, T const>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U const, T const>, T const>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U const, T const &&>, T const>);

    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U const &&, T>, T const &&>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U const &&, T &&>, T const &&>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U const &&, T const>, T const &&>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U const &&, T const &&>, T const &&>);

    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U, T>, T>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U, T &>, T>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U, volatile T>, T>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U, volatile T &>, T>);

    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U &, T>, T &>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U &, T &>, T &>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U &, volatile T>, T &>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<U &, volatile T &>, T &>);

    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<volatile U, T>, volatile T>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<volatile U, T &>, volatile T>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<volatile U, volatile T>, volatile T>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<volatile U, volatile T &>, volatile T>);

    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<volatile U &, T>, volatile T &>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<volatile U &, T &>, volatile T &>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<volatile U &, volatile T>, volatile T &>);
    STATIC_REQUIRE(std::same_as<ut::copy_cvref_t<volatile U &, volatile T &>, volatile T &>);
}
