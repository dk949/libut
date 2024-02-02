#include <catch.hpp>

#include <functional>
#include <iostream>
#include <limits>
#include <memory>

// ValuePtr is deprecated until it gets fixed, but I wanted to keep the tests around
#ifdef __GNUC__
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#else
#    pragma warning(push)
#    pragma warning(disable: 4996)
#endif

#ifdef assert
#    undef assert
#endif
#define assert REQUIRE
#include <ut/ptr_containers/valueptr.hpp>


using namespace ut;

TEST_CASE("ValuePtr: constructors", "[utils][ValuePtr]") {

    SECTION("default constructor") {
        ValuePtr<int> v;
        REQUIRE(*v == 0);
    }

    SECTION("value constructor") {
        ValuePtr v {37};
        REQUIRE(*v == 37);
    }

    SECTION("Move constructor") {
        auto v1 = ValuePtr {10};
        auto v2 = std::move(v1);
        REQUIRE(*v2 == 10);
    }

    SECTION("Copy constructor") {
        auto v1 = ValuePtr {10};
        auto v2 = v1;
        REQUIRE(*v2 == 10);
    }

    SECTION("implicit constructor") {
        ValuePtr<int> v = 3;
        REQUIRE(*v == 3);
    }

    SECTION("unique_ptr constructor") {
        std::unique_ptr<int> up = std::make_unique<int>(10);
        ValuePtr<int> v {std::move(up)};
        REQUIRE(*v == 10);
        SUCCEED("no leak and no double delete");
    }
}

TEST_CASE("ValuePtr: release", "[utils][ValuePtr]") {
    auto v = ValuePtr {27};
    auto ptr = v.release();
    REQUIRE(*ptr == 27);
    SUCCEED("no leak and no double delete");
}

TEST_CASE("ValuePtr: assignment", "[utils][ValuePtr]") {
    SECTION("Copy assignment") {
        ValuePtr<int> v1 = 10;
        ValuePtr<int> v2 = 12;
        v2 = v1;
        REQUIRE(*v2 == 10);
    }

    SECTION("Move assignment") {
        ValuePtr<int> v1 = 10;
        ValuePtr<int> v2 = 12;
        v2 = std::move(v1);
        REQUIRE(*v2 == 10);
    }
}

TEST_CASE("ValuePtr: comparison", "[utils][ValuePtr]") {
    auto i_same1 = 1;
    auto i_same2 = 1;
    auto i_diff1 = 100;
    auto i_diff2 = 100;

    auto same1 = ValuePtr(i_same1);
    auto same2 = ValuePtr(i_same2);
    auto diff1 = ValuePtr(i_diff1);
    auto diff2 = ValuePtr(i_diff2);

    REQUIRE(same1 == same1);
    REQUIRE(same1 == same2);
    REQUIRE(same2 == same1);
    REQUIRE(same1 != diff1);
    REQUIRE(same1 != diff2);

    REQUIRE(same1 == i_same1);
    REQUIRE(same1 == i_same2);
    REQUIRE(same2 == i_same1);
    REQUIRE(same1 != i_diff1);
    REQUIRE(same1 != i_diff2);

    REQUIRE(i_same1 == same1);
    REQUIRE(i_same1 == same2);
    REQUIRE(i_same2 == same1);
    REQUIRE(i_same1 != diff1);
    REQUIRE(i_same1 != diff2);

    REQUIRE_FALSE(same1 != same1);
    REQUIRE_FALSE(same1 != same2);
    REQUIRE_FALSE(same2 != same1);
    REQUIRE_FALSE(same1 == diff1);
    REQUIRE_FALSE(same1 == diff2);

    REQUIRE_FALSE(same1 != i_same1);
    REQUIRE_FALSE(same1 != i_same2);
    REQUIRE_FALSE(same2 != i_same1);
    REQUIRE_FALSE(same1 == i_diff1);
    REQUIRE_FALSE(same1 == i_diff2);

    REQUIRE_FALSE(i_same1 != same1);
    REQUIRE_FALSE(i_same1 != same2);
    REQUIRE_FALSE(i_same2 != same1);
    REQUIRE_FALSE(i_same1 == diff1);
    REQUIRE_FALSE(i_same1 == diff2);
}

TEST_CASE("ValuePtr: call operator", "[utils][ValuePtr]") {
    SECTION("function pointer") {
        using Fn = int(int);
        Fn *fp = [](int i) {
            REQUIRE(i == 42);
            return i + 1;
        };
        auto f1 = ValuePtr {fp};

        REQUIRE(f1(42) == 43);
    }

    SECTION("std::function") {
        std::function<int(int)> f = [](int i) {
            REQUIRE(i == 42);
            return i + 1;
        };
        auto f1 = ValuePtr {f};

        REQUIRE(f1(42) == 43);
    }

    SECTION("non capturing lambda") {
        auto f1 = ValuePtr {[](int i) {
            REQUIRE(i == 42);
            return i + 1;
        }};

        REQUIRE(f1(42) == 43);
    }

    SECTION("capturing lambda") {
        auto f1 = ValuePtr {[n = 3](int i) {
            REQUIRE(i == 42);
            return i + n;
        }};

        REQUIRE(f1(42) == 45);
    }
}

TEST_CASE("ValuePtr: stdlib integration", "[utils][ValuePtr]") {
    ValuePtr<int> v;

    SECTION("swap") {
        ValuePtr<int> std1 {10};
        ValuePtr<int> std2 {9};

        ValuePtr<int> sw1 {10};
        ValuePtr<int> sw2 {9};

        REQUIRE(std1 == sw1);
        REQUIRE(std2 == sw2);

        std::swap(std1, std2);
        REQUIRE(std1 == sw2);
        REQUIRE(std2 == sw1);

        sw1.swap(sw2);
        REQUIRE(std1 == sw1);
        REQUIRE(std2 == sw2);

        swap(std1, std2);
        REQUIRE(std1 == sw2);
        REQUIRE(std2 == sw1);
    }
}

TEST_CASE("ValuePtr: inheritance", "[utils][ValuePtr]") {
    struct Base {
        Base() = default;

        virtual bool isBase() {
            return true;
        }

        virtual ~Base() = default;
    };

    struct Derived : public Base {
        Derived()
                : Base() { }

        virtual bool isBase() override {
            return false;
        }
    };

    SECTION("copy construct") {
        ValuePtr<Base> vb {};
        ValuePtr<Derived> vd {};
        ValuePtr<Base> v1 = vb;
        ValuePtr<Base> v2 = vd;
        REQUIRE(v1->isBase());
        REQUIRE(!v2->isBase());
    }

    SECTION("copy construct value") {
        Base b;
        Derived d;
        ValuePtr<Base> v1 = b;
        ValuePtr<Base> v2 = d;
        REQUIRE(v1->isBase());
        REQUIRE(!v2->isBase());
    }

    SECTION("move construct") {
        ValuePtr<Base> vb {};
        ValuePtr<Derived> vd {};
        ValuePtr<Base> v1 = std::move(vb);
        ValuePtr<Base> v2 = std::move(vd);
        REQUIRE(v1->isBase());
        REQUIRE(!v2->isBase());
    }

    SECTION("move construct value") {
        Base b;
        Derived d;
        ValuePtr<Base> v1 = std::move(b);
        ValuePtr<Base> v2 = std::move(d);
        REQUIRE(v1->isBase());
        REQUIRE(!v2->isBase());
    }

    // NOTE: all of these tests assign Derived first then Base
    //       this is done to catch potential slicing

    SECTION("copy assign") {
        ValuePtr<Base> vb {};
        ValuePtr<Derived> vd {};
        ValuePtr<Base> v {};

        v = vd;
        REQUIRE(!v->isBase());
        v = vb;
        REQUIRE(v->isBase());

        // Make sure this is repeatable
        v = vd;
        REQUIRE(!v->isBase());
        v = vb;
        REQUIRE(v->isBase());
    }

    SECTION("copy assign value") {
        Base b {};
        Derived d {};
        ValuePtr<Base> v {};

        v = d;
        REQUIRE(!v->isBase());
        v = b;
        REQUIRE(v->isBase());

        // Make sure this is repeatable
        v = d;
        REQUIRE(!v->isBase());
        v = b;
        REQUIRE(v->isBase());
    }


    SECTION("move assign") {
        ValuePtr<Base> vb {};
        ValuePtr<Derived> vd {};

        ValuePtr<Base> v {};
        v = std::move(vd);
        REQUIRE(!v->isBase());
        v = std::move(vb);
        REQUIRE(v->isBase());
    }

    SECTION("move assign value") {
        Base b {};
        Derived d {};

        ValuePtr<Base> v {};
        v = std::move(d);
        REQUIRE(!v->isBase());
        v = std::move(b);
        REQUIRE(v->isBase());
    }
}

#ifdef __GNUC__
#    pragma GCC diagnostic pop
#else
#    pragma warning(pop)
#endif
