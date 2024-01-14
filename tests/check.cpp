#include <catch.hpp>
#include <ut/check/check.hpp>

#include <iostream>
#include <ranges>
#include <sstream>
#include <string_view>
#include <vector>

#define DFLT_CTOR(name) \
    name() noexcept { }
#define DTOR(name) \
    ~name() noexcept { }
#define COPY_CTOR(name) \
    name(name const &) noexcept { }
#define MOVE_CTOR(name) \
    name(name &&) noexcept { }
#define COPY_ASSGN(name)                     \
    name &operator=(name const &) noexcept { \
        return *this;                        \
    }
#define MOVE_ASSGN(name)                \
    name &operator=(name &&) noexcept { \
        return *this;                   \
    }

struct ThrowDfltCtor {

    ThrowDfltCtor() { }
    DTOR(ThrowDfltCtor)
    COPY_CTOR(ThrowDfltCtor)
    COPY_ASSGN(ThrowDfltCtor)
    MOVE_CTOR(ThrowDfltCtor)
    MOVE_ASSGN(ThrowDfltCtor)
};

struct ThrowDtor {
    DFLT_CTOR(ThrowDtor)

    // generally destructors are considered to be nothrow by default, since there's no way to catch an exception thrown
    // by a destructor (not actually sure if the standard mandates this).
    // But you can convince the compiler that the destructor is really nothrow
    ~ThrowDtor() noexcept(false) { }
    COPY_CTOR(ThrowDtor)
    COPY_ASSGN(ThrowDtor)
    MOVE_CTOR(ThrowDtor)
    MOVE_ASSGN(ThrowDtor)
};

struct ThrowCopyCtor {
    DFLT_CTOR(ThrowCopyCtor)
    DTOR(ThrowCopyCtor)

    ThrowCopyCtor(ThrowCopyCtor const &) { }
    COPY_ASSGN(ThrowCopyCtor)
    MOVE_CTOR(ThrowCopyCtor)
    MOVE_ASSGN(ThrowCopyCtor)
};

struct ThrowMoveCtor {
    DFLT_CTOR(ThrowMoveCtor)
    DTOR(ThrowMoveCtor)
    COPY_CTOR(ThrowMoveCtor)
    COPY_ASSGN(ThrowMoveCtor)

    ThrowMoveCtor(ThrowMoveCtor &&) { }
    MOVE_ASSGN(ThrowMoveCtor)
};

struct ThrowCopyAssgn {
    DFLT_CTOR(ThrowCopyAssgn)
    DTOR(ThrowCopyAssgn)
    COPY_CTOR(ThrowCopyAssgn)

    ThrowCopyAssgn &operator=(ThrowCopyAssgn const &) {
        return *this;
    }
    MOVE_CTOR(ThrowCopyAssgn)
    MOVE_ASSGN(ThrowCopyAssgn)
};

struct ThrowMoveAssgn {
    DFLT_CTOR(ThrowMoveAssgn)
    DTOR(ThrowMoveAssgn)
    COPY_CTOR(ThrowMoveAssgn)
    COPY_ASSGN(ThrowMoveAssgn)
    MOVE_CTOR(ThrowMoveAssgn)

    ThrowMoveAssgn &operator=(ThrowMoveAssgn &&) {
        return *this;
    }
};

TEST_CASE("Inheriting noexcept properties", "[check]") {
    // NOTE: it's important that `Check` inherits noexcept properties of the enclosed type because containers use it
    //       to determine if the value should be moved or copied

    using DfltCtor = ut::Check<ThrowDfltCtor>;
    using Dtor = ut::Check<ThrowDtor>;
    using CopyCtor = ut::Check<ThrowCopyCtor>;
    using MoveCtor = ut::Check<ThrowMoveCtor>;
    using CopyAssgn = ut::Check<ThrowCopyAssgn>;
    using MoveAssgn = ut::Check<ThrowMoveAssgn>;

#define DFLT_CTORABLE(T)  std::is_nothrow_default_constructible_v<T>
#define DTORABLE(T)       std::is_nothrow_destructible_v<T>
#define COPY_CTORABLE(T)  std::is_nothrow_copy_constructible_v<T>
#define MOVE_CTORABLE(T)  std::is_nothrow_move_constructible_v<T>
#define COPY_ASSGNABLE(T) std::is_nothrow_copy_assignable_v<T>
#define MOVE_ASSGNABLE(T) std::is_nothrow_move_assignable_v<T>

#define test(A, B)                                          \
    STATIC_REQUIRE(DFLT_CTORABLE(A) == DFLT_CTORABLE(B));   \
    STATIC_REQUIRE(DTORABLE(A) == DTORABLE(B));             \
    STATIC_REQUIRE(COPY_CTORABLE(A) == COPY_CTORABLE(B));   \
    STATIC_REQUIRE(MOVE_CTORABLE(A) == MOVE_CTORABLE(B));   \
    STATIC_REQUIRE(COPY_ASSGNABLE(A) == COPY_ASSGNABLE(B)); \
    STATIC_REQUIRE(MOVE_ASSGNABLE(A) == MOVE_ASSGNABLE(B))

    test(DfltCtor, ThrowDfltCtor);
    test(Dtor, ThrowDtor);
    test(CopyCtor, ThrowCopyCtor);
    test(MoveCtor, ThrowMoveCtor);
    test(CopyAssgn, ThrowCopyAssgn);
    test(MoveAssgn, ThrowMoveAssgn);
}

TEST_CASE("Check functionality", "[check]") {
    using namespace Catch::Matchers;
    std::stringstream ss;
    auto *cout_buf = std::cout.rdbuf(ss.rdbuf());
    {
        auto c1 = ut::ICheck {};  // ctor
        auto c2 = c1;             // copy ctor
        auto c3 = std::move(c1);  // move ctor
        c3 = c1;                  // copy assgn
        c1 = std::move(c3);       // move assgn
        // 3 dtors called
    }
    auto res = ss.str();
    // VERY IMPORTANT: return old buffer to cout before using REQUIRE
    std::cout.rdbuf(cout_buf);

    std::vector<std::string_view> vec;
    for (auto const line : std::views::split(res, '\n'))
        vec.emplace_back(line.begin(), line.end());

    REQUIRE(vec.size() == 9);
    // Using starts_with (instead of ==) because Check uses typeid name to identify objects
    REQUIRE(vec.at(0).starts_with("Check();"));
    REQUIRE(vec.at(1).starts_with("Check(const Check &);"));
    REQUIRE(vec.at(2).starts_with("Check(Check &&);"));
    REQUIRE(vec.at(3).starts_with("Check &operator=(Check);"));
    REQUIRE(vec.at(4).starts_with("Check &operator=(Check &&);"));
    REQUIRE(vec.at(5).starts_with("~Check();"));
    REQUIRE(vec.at(6).starts_with("~Check();"));
    REQUIRE(vec.at(7).starts_with("~Check();"));
    REQUIRE(vec.at(8) == "");
}
