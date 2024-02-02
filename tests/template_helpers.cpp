#include <catch.hpp>
#include <ut/ptr_containers/template_helpers.hpp>

using namespace ut::detail;

struct Base { };

struct SDerived : public Base { };

class CDerived : private Base { };

struct Unrelated { };

TEST_CASE("template_helpers: SameAsRemoveCVRef", "[utils][concepts]") {
    STATIC_REQUIRE(SameAsRemoveCVRef<int, int>);
    STATIC_REQUIRE(SameAsRemoveCVRef<int, int &>);
    STATIC_REQUIRE(SameAsRemoveCVRef<int &, int>);
    STATIC_REQUIRE(SameAsRemoveCVRef<int &, int &>);

    STATIC_REQUIRE(SameAsRemoveCVRef<int const, int>);
    STATIC_REQUIRE(SameAsRemoveCVRef<int const, int &>);
    STATIC_REQUIRE(SameAsRemoveCVRef<int const &, int>);
    STATIC_REQUIRE(SameAsRemoveCVRef<int const &, int &>);

    STATIC_REQUIRE(SameAsRemoveCVRef<int, int const>);
    STATIC_REQUIRE(SameAsRemoveCVRef<int, int const &>);
    STATIC_REQUIRE(SameAsRemoveCVRef<int &, int const>);
    STATIC_REQUIRE(SameAsRemoveCVRef<int &, int const &>);

    STATIC_REQUIRE(SameAsRemoveCVRef<int const, int const>);
    STATIC_REQUIRE(SameAsRemoveCVRef<int const, int const &>);
    STATIC_REQUIRE(SameAsRemoveCVRef<int const &, int const>);
    STATIC_REQUIRE(SameAsRemoveCVRef<int const &, int const &>);

    STATIC_REQUIRE(!SameAsRemoveCVRef<double, int>);
    STATIC_REQUIRE(!SameAsRemoveCVRef<double, int &>);
    STATIC_REQUIRE(!SameAsRemoveCVRef<double &, int>);
    STATIC_REQUIRE(!SameAsRemoveCVRef<double &, int &>);

    STATIC_REQUIRE(!SameAsRemoveCVRef<double const, int>);
    STATIC_REQUIRE(!SameAsRemoveCVRef<double const, int &>);
    STATIC_REQUIRE(!SameAsRemoveCVRef<double const &, int>);
    STATIC_REQUIRE(!SameAsRemoveCVRef<double const &, int &>);

    STATIC_REQUIRE(!SameAsRemoveCVRef<double, int const>);
    STATIC_REQUIRE(!SameAsRemoveCVRef<double, int const &>);
    STATIC_REQUIRE(!SameAsRemoveCVRef<double &, int const>);
    STATIC_REQUIRE(!SameAsRemoveCVRef<double &, int const &>);

    STATIC_REQUIRE(!SameAsRemoveCVRef<double const, int const>);
    STATIC_REQUIRE(!SameAsRemoveCVRef<double const, int const &>);
    STATIC_REQUIRE(!SameAsRemoveCVRef<double const &, int const>);
    STATIC_REQUIRE(!SameAsRemoveCVRef<double const &, int const &>);
}

TEST_CASE("template_helpers: DerivedOrEqualTo", "[utils][concepts]") {

    STATIC_REQUIRE(DerivedOrEqualTo<int, int>);
    STATIC_REQUIRE(DerivedOrEqualTo<int, int &>);
    STATIC_REQUIRE(DerivedOrEqualTo<int &, int>);
    STATIC_REQUIRE(DerivedOrEqualTo<int &, int &>);

    STATIC_REQUIRE(DerivedOrEqualTo<int const, int>);
    STATIC_REQUIRE(DerivedOrEqualTo<int const, int &>);
    STATIC_REQUIRE(DerivedOrEqualTo<int const &, int>);
    STATIC_REQUIRE(DerivedOrEqualTo<int const &, int &>);

    STATIC_REQUIRE(DerivedOrEqualTo<int, int const>);
    STATIC_REQUIRE(DerivedOrEqualTo<int, int const &>);
    STATIC_REQUIRE(DerivedOrEqualTo<int &, int const>);
    STATIC_REQUIRE(DerivedOrEqualTo<int &, int const &>);

    STATIC_REQUIRE(DerivedOrEqualTo<int const, int const>);
    STATIC_REQUIRE(DerivedOrEqualTo<int const, int const &>);
    STATIC_REQUIRE(DerivedOrEqualTo<int const &, int const>);
    STATIC_REQUIRE(DerivedOrEqualTo<int const &, int const &>);

    STATIC_REQUIRE(!DerivedOrEqualTo<double, int>);
    STATIC_REQUIRE(!DerivedOrEqualTo<double, int &>);
    STATIC_REQUIRE(!DerivedOrEqualTo<double &, int>);
    STATIC_REQUIRE(!DerivedOrEqualTo<double &, int &>);

    STATIC_REQUIRE(!DerivedOrEqualTo<double const, int>);
    STATIC_REQUIRE(!DerivedOrEqualTo<double const, int &>);
    STATIC_REQUIRE(!DerivedOrEqualTo<double const &, int>);
    STATIC_REQUIRE(!DerivedOrEqualTo<double const &, int &>);

    STATIC_REQUIRE(!DerivedOrEqualTo<double, int const>);
    STATIC_REQUIRE(!DerivedOrEqualTo<double, int const &>);
    STATIC_REQUIRE(!DerivedOrEqualTo<double &, int const>);
    STATIC_REQUIRE(!DerivedOrEqualTo<double &, int const &>);

    STATIC_REQUIRE(!DerivedOrEqualTo<double const, int const>);
    STATIC_REQUIRE(!DerivedOrEqualTo<double const, int const &>);
    STATIC_REQUIRE(!DerivedOrEqualTo<double const &, int const>);
    STATIC_REQUIRE(!DerivedOrEqualTo<double const &, int const &>);


    STATIC_REQUIRE(DerivedOrEqualTo<SDerived, Base>);
    STATIC_REQUIRE(DerivedOrEqualTo<SDerived, Base &>);
    STATIC_REQUIRE(DerivedOrEqualTo<SDerived &, Base>);
    STATIC_REQUIRE(DerivedOrEqualTo<SDerived &, Base &>);

    STATIC_REQUIRE(DerivedOrEqualTo<SDerived const, Base>);
    STATIC_REQUIRE(DerivedOrEqualTo<SDerived const, Base &>);
    STATIC_REQUIRE(DerivedOrEqualTo<SDerived const &, Base>);
    STATIC_REQUIRE(DerivedOrEqualTo<SDerived const &, Base &>);

    STATIC_REQUIRE(DerivedOrEqualTo<SDerived, Base const>);
    STATIC_REQUIRE(DerivedOrEqualTo<SDerived, Base const &>);
    STATIC_REQUIRE(DerivedOrEqualTo<SDerived &, Base const>);
    STATIC_REQUIRE(DerivedOrEqualTo<SDerived &, Base const &>);

    STATIC_REQUIRE(DerivedOrEqualTo<SDerived const, Base const>);
    STATIC_REQUIRE(DerivedOrEqualTo<SDerived const, Base const &>);
    STATIC_REQUIRE(DerivedOrEqualTo<SDerived const &, Base const>);
    STATIC_REQUIRE(DerivedOrEqualTo<SDerived const &, Base const &>);


    STATIC_REQUIRE(!DerivedOrEqualTo<Base, SDerived>);
    STATIC_REQUIRE(!DerivedOrEqualTo<Base, SDerived &>);
    STATIC_REQUIRE(!DerivedOrEqualTo<Base &, SDerived>);
    STATIC_REQUIRE(!DerivedOrEqualTo<Base &, SDerived &>);

    STATIC_REQUIRE(!DerivedOrEqualTo<Base const, SDerived>);
    STATIC_REQUIRE(!DerivedOrEqualTo<Base const, SDerived &>);
    STATIC_REQUIRE(!DerivedOrEqualTo<Base const &, SDerived>);
    STATIC_REQUIRE(!DerivedOrEqualTo<Base const &, SDerived &>);

    STATIC_REQUIRE(!DerivedOrEqualTo<Base, SDerived const>);
    STATIC_REQUIRE(!DerivedOrEqualTo<Base, SDerived const &>);
    STATIC_REQUIRE(!DerivedOrEqualTo<Base &, SDerived const>);
    STATIC_REQUIRE(!DerivedOrEqualTo<Base &, SDerived const &>);

    STATIC_REQUIRE(!DerivedOrEqualTo<Base const, SDerived const>);
    STATIC_REQUIRE(!DerivedOrEqualTo<Base const, SDerived const &>);
    STATIC_REQUIRE(!DerivedOrEqualTo<Base const &, SDerived const>);
    STATIC_REQUIRE(!DerivedOrEqualTo<Base const &, SDerived const &>);


    STATIC_REQUIRE(!DerivedOrEqualTo<CDerived, Base>);
    STATIC_REQUIRE(!DerivedOrEqualTo<CDerived, Base &>);
    STATIC_REQUIRE(!DerivedOrEqualTo<CDerived &, Base>);
    STATIC_REQUIRE(!DerivedOrEqualTo<CDerived &, Base &>);

    STATIC_REQUIRE(!DerivedOrEqualTo<CDerived const, Base>);
    STATIC_REQUIRE(!DerivedOrEqualTo<CDerived const, Base &>);
    STATIC_REQUIRE(!DerivedOrEqualTo<CDerived const &, Base>);
    STATIC_REQUIRE(!DerivedOrEqualTo<CDerived const &, Base &>);

    STATIC_REQUIRE(!DerivedOrEqualTo<CDerived, Base const>);
    STATIC_REQUIRE(!DerivedOrEqualTo<CDerived, Base const &>);
    STATIC_REQUIRE(!DerivedOrEqualTo<CDerived &, Base const>);
    STATIC_REQUIRE(!DerivedOrEqualTo<CDerived &, Base const &>);

    STATIC_REQUIRE(!DerivedOrEqualTo<CDerived const, Base const>);
    STATIC_REQUIRE(!DerivedOrEqualTo<CDerived const, Base const &>);
    STATIC_REQUIRE(!DerivedOrEqualTo<CDerived const &, Base const>);
    STATIC_REQUIRE(!DerivedOrEqualTo<CDerived const &, Base const &>);


    STATIC_REQUIRE(!DerivedOrEqualTo<Unrelated, Base>);
    STATIC_REQUIRE(!DerivedOrEqualTo<Unrelated, Base &>);
    STATIC_REQUIRE(!DerivedOrEqualTo<Unrelated &, Base>);
    STATIC_REQUIRE(!DerivedOrEqualTo<Unrelated &, Base &>);

    STATIC_REQUIRE(!DerivedOrEqualTo<Unrelated const, Base>);
    STATIC_REQUIRE(!DerivedOrEqualTo<Unrelated const, Base &>);
    STATIC_REQUIRE(!DerivedOrEqualTo<Unrelated const &, Base>);
    STATIC_REQUIRE(!DerivedOrEqualTo<Unrelated const &, Base &>);

    STATIC_REQUIRE(!DerivedOrEqualTo<Unrelated, Base const>);
    STATIC_REQUIRE(!DerivedOrEqualTo<Unrelated, Base const &>);
    STATIC_REQUIRE(!DerivedOrEqualTo<Unrelated &, Base const>);
    STATIC_REQUIRE(!DerivedOrEqualTo<Unrelated &, Base const &>);

    STATIC_REQUIRE(!DerivedOrEqualTo<Unrelated const, Base const>);
    STATIC_REQUIRE(!DerivedOrEqualTo<Unrelated const, Base const &>);
    STATIC_REQUIRE(!DerivedOrEqualTo<Unrelated const &, Base const>);
    STATIC_REQUIRE(!DerivedOrEqualTo<Unrelated const &, Base const &>);
}

TEST_CASE("template_helpers: AddTransitiveConst", "[utils][concepts]") {
    using Type = int;
    using TypeConst = int const;

    using TypePtr = int *;
    using TypePtrConst = int *const;
    using TypeConstPtr = int const *;
    using TypeConstPtrConst = int const *const;

    using TypePtrPtr = int **;
    using TypePtrPtrConst = int **const;
    using TypePtrConstPtr = int *const *;
    using TypePtrConstPtrConst = int *const *const;
    using TypeConstPtrPtr = int const **;
    using TypeConstPtrPtrConst = int const **const;
    using TypeConstPtrConstPtr = int const *const *;
    using TypeConstPtrConstPtrConst = int const *const *const;


    STATIC_REQUIRE(std::is_same_v<AddTransitiveConstT<Type>, TypeConst>);
    STATIC_REQUIRE(std::is_same_v<AddTransitiveConstT<TypeConst>, TypeConst>);

    using TypeConstPtrConst = int const *const;

    STATIC_REQUIRE(std::is_same_v<AddTransitiveConstT<TypePtr>, TypeConstPtrConst>);
    STATIC_REQUIRE(std::is_same_v<AddTransitiveConstT<TypePtrConst>, TypeConstPtrConst>);
    STATIC_REQUIRE(std::is_same_v<AddTransitiveConstT<TypeConstPtr>, TypeConstPtrConst>);
    STATIC_REQUIRE(std::is_same_v<AddTransitiveConstT<TypeConstPtrConst>, TypeConstPtrConst>);

    using TypeConstPtrConstPtrConst = int const *const *const;

    STATIC_REQUIRE(std::is_same_v<AddTransitiveConstT<TypePtrPtr>, TypeConstPtrConstPtrConst>);
    STATIC_REQUIRE(std::is_same_v<AddTransitiveConstT<TypePtrPtrConst>, TypeConstPtrConstPtrConst>);
    STATIC_REQUIRE(std::is_same_v<AddTransitiveConstT<TypePtrConstPtr>, TypeConstPtrConstPtrConst>);
    STATIC_REQUIRE(std::is_same_v<AddTransitiveConstT<TypePtrConstPtrConst>, TypeConstPtrConstPtrConst>);
    STATIC_REQUIRE(std::is_same_v<AddTransitiveConstT<TypeConstPtrPtr>, TypeConstPtrConstPtrConst>);
    STATIC_REQUIRE(std::is_same_v<AddTransitiveConstT<TypeConstPtrPtrConst>, TypeConstPtrConstPtrConst>);
    STATIC_REQUIRE(std::is_same_v<AddTransitiveConstT<TypeConstPtrConstPtr>, TypeConstPtrConstPtrConst>);
    STATIC_REQUIRE(std::is_same_v<AddTransitiveConstT<TypeConstPtrConstPtrConst>, TypeConstPtrConstPtrConst>);
}
