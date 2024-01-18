#include <catch.hpp>
using namespace Catch::Matchers;
#include <ut/demangle/demangle.hpp>

// NOTE: since this function depends on compiler extensions and platform define behaviour, it's quite hard to
// test. These test just ensure it returns something remotely sensible.



namespace Lorem {
struct Ipsum {
    class Dolor { };
};
};

class Base {
public:
    Base() = default;
    virtual ~Base() = default;
};

class Derived : public Base { };

TEST_CASE("Demangle simple", "[demangle]") {
    auto const i1 = ut::typeNameDynamic(1);
    constexpr auto i2 = ut::typeName<int>();

    auto const d1 = ut::typeNameDynamic(2.3);
    constexpr auto d2 = ut::typeName<double>();

    REQUIRE_THAT(i1, ContainsSubstring("int"));
    REQUIRE_THAT(std::string(i2), ContainsSubstring("int"));
    REQUIRE_THAT(d1, ContainsSubstring("double"));
    REQUIRE_THAT(std::string(d2), ContainsSubstring("double"));
}

TEST_CASE("Demangle complex", "[demangle]") {
    auto const i1 = ut::typeNameDynamic(Lorem::Ipsum {});
    constexpr auto i2 = ut::typeName<Lorem::Ipsum>();

    auto const d1 = ut::typeNameDynamic(Lorem::Ipsum::Dolor {});
    constexpr auto d2 = ut::typeName<Lorem::Ipsum::Dolor>();

    REQUIRE(i1 == i2);
    REQUIRE(d1 == d2);
    REQUIRE_THAT(i1, ContainsSubstring("Lorem") && ContainsSubstring("Ipsum"));
    REQUIRE_THAT(std::string(i2), ContainsSubstring("Lorem") && ContainsSubstring("Ipsum"));
    REQUIRE_THAT(d1, ContainsSubstring("Lorem") && ContainsSubstring("Ipsum") && ContainsSubstring("Dolor"));
    REQUIRE_THAT(std::string(d2), ContainsSubstring("Lorem") && ContainsSubstring("Ipsum") && ContainsSubstring("Dolor"));
}

TEST_CASE("Demangle const and references", "[demangle]") {

    // clang-format off
    int i = 0;
    int const i_c = 0;
    const int c_i = 0;
    Lorem::Ipsum ipsum {};
    int &i_ref = i;
    int const &i_cref = i;
    const int &c_i_ref = i;
    Lorem::Ipsum &ipsum_ref = ipsum;
    Lorem::Ipsum const &ipsum_cref = ipsum;

    int &&i_ref_ref = std::move(i);
    int const &&i_cref_ref = std::move(i);
    const int &&c_i_ref_ref = std::move(i);
    Lorem::Ipsum &&ipsum_ref_ref = std::move(ipsum);
    Lorem::Ipsum const &&ipsum_cref_ref = std::move(ipsum);
    // clang-format on
    STATIC_REQUIRE(ut::typeName<decltype(i_c)>() == ut::typeName<decltype(c_i)>());
    STATIC_REQUIRE(ut::typeName<decltype(i)>() != ut::typeName<decltype(i_ref)>());
    STATIC_REQUIRE(ut::typeName<decltype(i_cref)>() == ut::typeName<decltype(c_i_ref)>());
    STATIC_REQUIRE(ut::typeName<decltype(i_cref_ref)>() == ut::typeName<decltype(c_i_ref_ref)>());
    STATIC_REQUIRE(ut::typeName<decltype(ipsum)>() != ut::typeName<decltype(ipsum_ref)>());
    STATIC_REQUIRE(ut::typeName<decltype(ipsum)>() != ut::typeName<decltype(ipsum_ref_ref)>());

#define REQ_REGEX(sv, re) REQUIRE_THAT(std::string(sv), re)
    // GCC and clang use west const, msvc uses east const
    // so have to have two matchers for the const cases

    REQ_REGEX(ut::typeName<decltype(i_c)>(),
        Matches(R"([ \t]*int [^c]*const)")  //
            || Matches(R"([\t ]*const[ \t]+int[ \t]*)"));
    REQ_REGEX(ut::typeName<decltype(i_ref)>(), Matches(R"([ \t]*int[^&]*&)"));
    REQ_REGEX(ut::typeName<decltype(i_cref)>(),
        Matches(R"([ \t]*int [^c]*const[^&]*&)")  //
            || Matches(R"([ \t]*const[ \t]+int[ \t]*[^&]*&)"));
    REQ_REGEX(ut::typeName<decltype(ipsum_ref)>(), Matches(R"([ \t]*Lorem.*Ipsum[^&]*&)"));
    REQ_REGEX(ut::typeName<decltype(ipsum_cref)>(),
        Matches(R"([ \t]*Lorem.*Ipsum [^c]*const[^&]*&)")  //
            || Matches(R"([ \t]*const[ \t]+Lorem.*Ipsum[^&]*&)"));
    REQ_REGEX(ut::typeName<decltype(i_ref_ref)>(), Matches(R"([ \t]*int[^&]*&&)"));
    REQ_REGEX(ut::typeName<decltype(i_cref_ref)>(),
        Matches(R"([ \t]*int [^c]*const[^&]*&&)")  //
            || Matches(R"([ \t]*const[ \t]+int[^&]*&&)"));
    REQ_REGEX(ut::typeName<decltype(ipsum_ref_ref)>(), Matches(R"([ \t]*Lorem.*Ipsum[^&]*&&)"));
    REQ_REGEX(ut::typeName<decltype(ipsum_cref_ref)>(),
        Matches(R"([ \t]*Lorem.*Ipsum [^c]*const[^&]*&&)")  //
            || Matches(R"([ \t]*const[ \t]+Lorem.*Ipsum[^&]*&&)"));
}

TEST_CASE("Demangle inheritance", "[demangle]") {

    Base b {};
    Derived d {};

    Base &b_ref = b;
    Base &d_ref = d;

    STATIC_REQUIRE(ut::typeName<decltype(b)> != ut::typeName<decltype(d)>);
    STATIC_REQUIRE(ut::typeName<decltype(b_ref)> == ut::typeName<decltype(d_ref)>);
    REQUIRE_THAT(ut::typeNameDynamic(b_ref), ContainsSubstring("Base") && !ContainsSubstring("Derived"));
    REQUIRE_THAT(ut::typeNameDynamic(d_ref), !ContainsSubstring("Base") && ContainsSubstring("Derived"));
}
