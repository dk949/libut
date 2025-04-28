#include <catch.hpp>
#include <ut/pack_loops/pack_loops.hpp>

#include <type_traits>

template<typename... T>
void loopOverArgs(T... args) {
    int count = 0;
    UT_PACK_FOR(int i, args, {
        REQUIRE(i == count);
        count++;
    });
    REQUIRE(count == 4);
}

template<typename... T>
void loopOverArgsBreak(T... args) {
    int count = 0;
    UT_PACK_FOR(int i, args, {
        REQUIRE(i == count);
        count++;
        if (count == 2) UT_PACK_BREAK;
    });
    REQUIRE(count == 2);
}

template<typename... T>
void loopOverArgsContinue(T... args) {
    int count = 0;
    UT_PACK_FOR(int i, args, {
        if (i < 0) UT_PACK_CONTINUE;
        REQUIRE(i == count);
        count++;
    });
    REQUIRE(count == 4);
}

template<int... ints>
void loopOverParameterPack() {
    int count = 0;
    UT_PACK_FOR(int i, ints, {
        REQUIRE(i == count);
        count++;
    });
    REQUIRE(count == 4);
}

template<int... ints>
void loopOverParameterPackBreak() {
    int count = 0;
    UT_PACK_FOR(int i, ints, {
        REQUIRE(i == count);
        count++;
        if (count == 2) UT_PACK_BREAK;
    });
    REQUIRE(count == 2);
}

template<int... ints>
void loopOverParameterPackContinue() {
    int count = 0;
    UT_PACK_FOR(int i, ints, {
        if (i < 0) UT_PACK_CONTINUE;
        REQUIRE(i == count);
        count++;
    });
    REQUIRE(count == 4);
}

template<typename... Ts>
void loopOverTypes() {
    int count = 0;
    int float_count = 0;
    UT_PACK_FOR_T(I, Ts, {
        count++;
        if constexpr (std::is_same_v<I, float>) {
            float_count++;
        }
    });
    REQUIRE(count == 4);
    REQUIRE(float_count == 2);
}

template<typename... Ts>
void loopOverTypesBreak() {
    int count = 0;
    int float_count = 0;
    UT_PACK_FOR_T(I, Ts, {
        count++;
        if constexpr (std::is_same_v<I, float>) {
            float_count++;
        } else if constexpr (std::is_same_v<I, double>) {
            UT_PACK_BREAK;
        }
    });
    REQUIRE(count == 4);
    REQUIRE(float_count == 2);
}

template<typename... Ts>
void loopOverTypesContinue() {
    int count = 0;
    int float_count = 0;
    UT_PACK_FOR_T(I, Ts, {
        if constexpr (std::is_same_v<I, float>) {
            float_count++;
        } else if constexpr (std::is_same_v<I, double>) {
            UT_PACK_CONTINUE;
        }
        count++;
    });
    REQUIRE(count == 4);
    REQUIRE(float_count == 2);
}

template<typename... T>
int indexArgs(T... args) {
    return UT_PACK_IDX(args, 2);
}

template<typename... T>
int indexAndModifyArgs(T... args) {
    UT_PACK_IDX_MUT(args, 2)++;
    return UT_PACK_IDX(args, 2);
}

template<int... ints>
int indexPack() {
    return UT_PACK_IDX(ints, 2);
}

TEST_CASE("pack for loop", "[pack_loops]") {
    loopOverArgs(0, 1, 2, 3);
    loopOverArgsBreak(0, 1, 2, 3);
    loopOverArgsContinue(0, -100, 1, -100, 2, -100, 3);
    loopOverParameterPack<0, 1, 2, 3>();
    loopOverParameterPackBreak<0, 1, 2, 3>();
    loopOverParameterPackContinue<0, -100, 1, -100, 2, -100, 3>();
}

TEST_CASE("type pack for loop", "[pack_loops]") {
    loopOverTypes<int, float, double, float>();
    loopOverTypesBreak<int, float, float, double, float, int>();
    loopOverTypesContinue<int, float, double, float, int>();
}

TEST_CASE("pack index", "[pack_loops]") {
    REQUIRE(indexArgs(0, 0, 3, 0) == 3);
    REQUIRE(indexAndModifyArgs(0, 0, 3, 0) == 4);
    REQUIRE(indexPack<0, 0, 3, 0>() == 3);
}
