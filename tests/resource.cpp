#include <catch.hpp>
#include <ut/resource/resource.hpp>

struct Res { };

bool func_released = false;

void release(Res const &) {
    func_released = true;
}

TEST_CASE("Value resouce initialisation", "[resource]") {
    SECTION("lambda") {
        bool released = false;

        auto release_lambda = [&](Res) {
            released = true;
        };

        { ut::Resource<Res, decltype(release_lambda)> resource {Res {}, release_lambda}; }
        REQUIRE(released);
    }
    SECTION("function") {
        func_released = false;
        { ut::Resource<Res, decltype(release) *> resource {Res {}, &release}; }
        REQUIRE(func_released);
    }
}

TEST_CASE("Delayed value initialisatoin", "[resource]") {

    bool released = false;

    auto release_lambda = [&](Res) {
        released = true;
    };
    { ut::Resource<Res, decltype(release_lambda)> r {release_lambda}; }
    REQUIRE(!released);
    {
        ut::Resource<Res, decltype(release_lambda)> r {release_lambda};
        r.acquire(Res {});
    }
    REQUIRE(released);
}

TEST_CASE("resource ctad", "[resource]") {
    bool released = false;

    auto release_lambda = [&](Res) {
        released = true;
    };
    SECTION("both arsg lambda") {
        { ut::Resource r {Res {}, release_lambda}; }
        REQUIRE(released);
    }
    SECTION("both args function") {
        func_released = false;
        { ut::Resource resource {Res {}, &release}; }
        REQUIRE(func_released);
    }
    SECTION("single arg lambda") {
        {
            ut::Resource r {release_lambda};
            r.acquire(Res {});
        }
        REQUIRE(released);
    }
    SECTION("single arg function") {
        func_released = false;
        {
            ut::Resource r {release};
            r.acquire(Res {});
        }
        REQUIRE(func_released);
    }
}
