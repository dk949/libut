#include <catch.hpp>
#include <catch2/catch_test_macros.hpp>
#include <ut/change_observer/change_observer.hpp>

#include <sstream>
#include <type_traits>
#include <utility>

struct Simple {
    int x = 1;
    float y = 2.2f;
    std::string_view z = "three";

    int getX() const {
        return x;
    }

    void setX(int xx) {
        x = xx;
    }
};

struct NoMove {
    bool operator==(NoMove const &) const = default;
    int x = 3, y = 4;
    NoMove() = default;

    NoMove(int xx, int yy)
            : x(xx)
            , y(yy) { }

    NoMove(NoMove const &) = default;
    NoMove(NoMove &&) = delete;
    NoMove &operator=(NoMove const &) = default;
    NoMove &operator=(NoMove &&) = delete;
    ~NoMove() = default;

    int getX() const {
        return x;
    }

    void setX(int xx) {
        x = xx;
    }
};

struct NoCopy {
    float x = 10.7f, y = 15.9f;

    bool operator==(NoCopy const &) const = default;

    NoCopy(float xx, float yy)
            : x(xx)
            , y(yy) { }

    NoCopy() = default;
    NoCopy(NoCopy const &) = delete;
    NoCopy(NoCopy &&) = default;
    NoCopy &operator=(NoCopy const &) = delete;
    NoCopy &operator=(NoCopy &&) = default;
    ~NoCopy() = default;

    float getX() const {
        return x;
    }

    void setX(float xx) {
        x = xx;
    }
};

template<typename T>
T getValue();

template<>
int getValue<int>() {
    return 43;
}

template<>
Simple getValue<Simple>() {
    return Simple {42, 42.f, "42"};
}

template<>
NoMove getValue<NoMove>() {
    return {7, 10};
}

template<>
NoCopy getValue<NoCopy>() {
    return {1, 2};
}

template<typename T>
T getDefaultValue();

template<>
int getDefaultValue<int>() {
    return 0;
}

template<>
Simple getDefaultValue<Simple>() {
    return Simple {1, 2.2f, "three"};
}

template<>
NoMove getDefaultValue<NoMove>() {
    return {3, 4};
}

template<>
NoCopy getDefaultValue<NoCopy>() {
    return {10.7f, 15.9f};
}

template<typename T>
T getSimilarValue();

template<>
int getSimilarValue<int>() {
    return 45;
}

template<>
Simple getSimilarValue<Simple>() {
    return Simple {42, 43.f, "44"};
}

template<>
NoMove getSimilarValue<NoMove>() {
    return {7, 11};
}

template<>
NoCopy getSimilarValue<NoCopy>() {
    return {1, 3};
}

template<typename T>
bool compare(T const &a, T const &b);

template<>
bool compare<int>(int const &a, int const &b) {
    return a == b;
}

template<>
bool compare<Simple>(Simple const &a, Simple const &b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

template<>
bool compare<NoMove>(NoMove const &a, NoMove const &b) {
    return a.x == b.x && a.y == b.y;
}

template<>
bool compare<NoCopy>(NoCopy const &a, NoCopy const &b) {
    return a.x == b.x && a.y == b.y;
}

template<typename T>
struct CustomCompare;

template<>
struct CustomCompare<int> {
    bool operator()(int a, int b) {
        return a % 2 == b % 2;
    }
};

template<>
struct CustomCompare<Simple> {
    bool operator()(Simple const &a, Simple const &b) {
        return a.x == b.x;
    }
};

template<>
struct CustomCompare<NoMove> {
    bool operator()(NoMove const &a, NoMove const &b) {
        return a.x == b.x;
    }
};

template<>
struct CustomCompare<NoCopy> {
    bool operator()(NoCopy const &a, NoCopy const &b) {
        return a.x == b.x;
    }
};

template<typename T>
std::string toString(T const &);

template<>
std::string toString<int>(int const &v) {
    std::stringstream ss;
    ss << "int(" << v << ")";
    return ss.str();
}

template<>
std::string toString<Simple>(Simple const &v) {
    std::stringstream ss;
    ss << "Simple(" << v.x << ", " << v.y << ", " << v.z << ")";
    return ss.str();
}

template<>
std::string toString<NoMove>(NoMove const &v) {
    std::stringstream ss;
    ss << "NoMove(" << v.x << ", " << v.y << ")";
    return ss.str();
}

template<>
std::string toString<NoCopy>(NoCopy const &v) {
    std::stringstream ss;
    ss << "NoCopy(" << v.x << ", " << v.y << ")";
    return ss.str();
}

TEMPLATE_TEST_CASE("Change observer KeepOldCopy::Yes",
    "[change_observer]",
    // NoMove,  //
    // NoCopy,  //
    int,    //
    Simple  //
) {
    SECTION("No compare") {
        using Observer = ut::ChangeObserver<TestType, ut::ChangeObserverKeepOldCopy::Yes, void>;
        SECTION("Default ctor & update") {
            int changed = 0;
            Observer default_ctor;
            default_ctor.onChange([&](auto &&...) { changed++; });
            default_ctor.onChange([&](TestType const &v1, TestType const &v2) {
                if (changed == 1) {
                    REQUIRE(compare(v1, getDefaultValue<TestType>()));
                } else {
                    REQUIRE(compare(v1, getValue<TestType>()));
                }
                REQUIRE(compare(v2, getValue<TestType>()));
                changed++;
            });
            default_ctor.update(getValue<TestType>());
            REQUIRE(changed == 2);
            default_ctor.update(getValue<TestType>());
            REQUIRE(changed == 4);  // with no comparator, all updates invoke the callbacks
        }
        SECTION("Value ctor & Assignment") {
            int changed = 0;
            Observer value_ctor {getValue<TestType>()};
            value_ctor.onChange([&](auto &&...) { changed++; });
            Observer default_val;
            Observer similar_val {getSimilarValue<TestType>()};
            value_ctor = default_val;
            value_ctor = Observer {getValue<TestType>()};
            value_ctor = Observer {getValue<TestType>()};
            REQUIRE(changed == 3);
        }
        SECTION("getRef & Proxy") {
            int changed = 0;
            Observer v;
            v.onChange([&](auto &&...) { changed++; });
            v.getRef() = getValue<TestType>();
            REQUIRE(changed == 1);
            {
                auto ref = v.getRef();
                ref = getDefaultValue<TestType>();
                REQUIRE(changed == 1);  // No calls until ref goes out of scope
            }
            REQUIRE(changed == 2);
            v.getRef() = getValue<TestType>();
            REQUIRE(changed == 3);
            {
                // Setting the
                auto ref = v.getRef();
                ref = getDefaultValue<TestType>();
                ref = getValue<TestType>();
                ref = getDefaultValue<TestType>();
                ref = getValue<TestType>();
            }
            REQUIRE(changed == 4);
            REQUIRE(compare(v.get(), getValue<TestType>()));
            if constexpr (requires { v.get().getX(); }) {
                v.getRef()->setX(55);
                REQUIRE(changed == 5);
                v.getRef()->getX();
                REQUIRE(changed == 6);  // Still counts as an update
                std::as_const(v).getRef()->getX();
                REQUIRE(changed == 6);  // no call
                v.getCRef()->getX();
                REQUIRE(changed == 6);  // no call
            }
        }
    }
    SECTION("Default compare") {
        if constexpr (requires { std::equal_to<> {}(std::declval<TestType>(), std::declval<TestType>()); }) {
            using Observer = ut::ChangeObserver<TestType, ut::ChangeObserverKeepOldCopy::Yes, std::equal_to<>>;
            SECTION("Default ctor & update") {
                int changed = 0;
                Observer default_ctor;
                default_ctor.onChange([&](auto &&...) { changed++; });
                default_ctor.onChange([&](TestType const &v1, TestType const &v2) {
                    if (changed == 1) {
                        REQUIRE(compare(v1, getDefaultValue<TestType>()));
                    } else {
                        FAIL("This should never happen: v1 = " << toString(v1) << ", v2 = " << toString(v2));
                    }
                    REQUIRE(compare(v2, getValue<TestType>()));
                    changed++;
                });
                default_ctor.update(getValue<TestType>());
                REQUIRE(changed == 2);
                default_ctor.update(getValue<TestType>());
                REQUIRE(changed == 2);
            }
            SECTION("Value ctor & Assignment") {
                int changed = 0;
                Observer value_ctor {getValue<TestType>()};
                value_ctor.onChange([&](auto &&...) { changed++; });
                Observer default_val;
                Observer similar_val {getSimilarValue<TestType>()};
                value_ctor = default_val;
                value_ctor = Observer {getValue<TestType>()};
                value_ctor = Observer {getValue<TestType>()};
                REQUIRE(changed == 2);
            }
            SECTION("getRef & Proxy") {
                int changed = 0;
                Observer v;
                v.onChange([&](auto &&...) { changed++; });
                v.getRef() = getValue<TestType>();
                REQUIRE(changed == 1);
                {
                    auto ref = v.getRef();
                    ref = getDefaultValue<TestType>();
                    REQUIRE(changed == 1);  // No calls until ref goes out of scope
                }
                REQUIRE(changed == 2);
                v.getRef() = getValue<TestType>();
                REQUIRE(changed == 3);
                {
                    // Setting the
                    auto ref = v.getRef();
                    ref = getDefaultValue<TestType>();
                    ref = getValue<TestType>();
                    ref = getDefaultValue<TestType>();
                    ref = getValue<TestType>();
                }
                REQUIRE(changed == 3);  // no call
                REQUIRE(compare(v.get(), getValue<TestType>()));
                if constexpr (requires { v.get().getX(); }) {
                    v.getRef()->setX(55);
                    REQUIRE(changed == 4);
                    std::as_const(v).getRef()->getX();
                    REQUIRE(changed == 4);  // no call
                    v.getCRef()->getX();
                    REQUIRE(changed == 4);  // no call
                }
            }
        }
    }
    SECTION("Custom compare") {
        using Observer = ut::ChangeObserver<TestType, ut::ChangeObserverKeepOldCopy::Yes, CustomCompare<TestType>>;
        SECTION("Default ctor & update") {
            int changed = 0;
            Observer default_ctor;
            default_ctor.onChange([&](auto &&...) { changed++; });
            default_ctor.onChange([&](TestType const &v1, TestType const &v2) {  //
                switch (changed) {
                    case 1: {
                        REQUIRE(compare(v1, getDefaultValue<TestType>()));
                        REQUIRE(compare(v2, getValue<TestType>()));
                        break;
                    }
                    case 3: {
                        REQUIRE(compare(v1, getSimilarValue<TestType>()));
                        REQUIRE(compare(v2, getDefaultValue<TestType>()));
                        break;
                    }
                    default: FAIL("This should never happen: v1 = " << toString(v1) << ", v2 = " << toString(v2));
                }
                changed++;
            });
            default_ctor.update(getValue<TestType>());
            REQUIRE(changed == 2);
            default_ctor.update(getSimilarValue<TestType>());  // This replaces the value, but does not
                                                               // trigger the callbacks
            REQUIRE(changed == 2);
            default_ctor.update(getDefaultValue<TestType>());
            REQUIRE(changed == 4);
        }
        SECTION("Value ctor & Assignment") {
            int changed = 0;
            Observer value_ctor {getValue<TestType>()};
            value_ctor.onChange([&](auto &&...) { changed++; });
            Observer default_val;
            Observer similar_val {getSimilarValue<TestType>()};
            value_ctor = default_val;
            value_ctor = std::move(similar_val);
            value_ctor = Observer {getValue<TestType>()};
            REQUIRE(changed == 2);
        }
        SECTION("getRef & Proxy") {
            int changed = 0;
            Observer v;
            v.onChange([&](auto &&...) { changed++; });
            v.getRef() = getValue<TestType>();
            REQUIRE(changed == 1);
            {
                auto ref = v.getRef();
                ref = getDefaultValue<TestType>();
                REQUIRE(changed == 1);  // No calls until ref goes out of scope
            }
            REQUIRE(changed == 2);
            v.getRef() = getValue<TestType>();
            REQUIRE(changed == 3);
            {
                // Setting the
                auto ref = v.getRef();
                ref = getDefaultValue<TestType>();
                ref = getSimilarValue<TestType>();
                ref = getDefaultValue<TestType>();
                ref = getSimilarValue<TestType>();
            }
            REQUIRE(changed == 3);  // no call
            REQUIRE(compare(v.get(), getSimilarValue<TestType>()));
            if constexpr (requires { v.get().getX(); }) {
                v.getRef()->setX(55);
                REQUIRE(changed == 4);
                std::as_const(v).getRef()->getX();
                REQUIRE(changed == 4);  // no call
                v.getCRef()->getX();
                REQUIRE(changed == 4);  // no call
            }
        }
    }
}

TEMPLATE_TEST_CASE("Change observer KeepOldCopy::No",
    "[change_observer]",
    // NoMove,//
    // NoCopy, //
    int,    //
    Simple  //
) {
    SECTION("No compare") {
        using Observer = ut::ChangeObserver<TestType, ut::ChangeObserverKeepOldCopy::No, void>;
        SECTION("Default ctor & update") {
            int changed = 0;
            Observer default_ctor;
            default_ctor.onChange([&](auto &&...) { changed++; });
            default_ctor.onChange([&](TestType const &v) {
                REQUIRE(compare(v, getValue<TestType>()));
                changed++;
            });
            default_ctor.update(getValue<TestType>());
            REQUIRE(changed == 2);
            default_ctor.update(getValue<TestType>());
            REQUIRE(changed == 4);  // with no comparator, all updates invoke the callbacks
        }
        SECTION("Value ctor & Assignment") {
            int changed = 0;
            Observer value_ctor {getValue<TestType>()};
            value_ctor.onChange([&](auto &&...) { changed++; });
            Observer default_val;
            Observer similar_val {getSimilarValue<TestType>()};
            value_ctor = default_val;
            value_ctor = Observer {getValue<TestType>()};
            value_ctor = Observer {getValue<TestType>()};
            REQUIRE(changed == 3);
        }
        SECTION("getRef & Proxy") {
            int changed = 0;
            Observer v;
            v.onChange([&](auto &&...) { changed++; });
            v.getRef() = getValue<TestType>();
            REQUIRE(changed == 1);
            {
                auto ref = v.getRef();
                ref = getDefaultValue<TestType>();
                REQUIRE(changed == 1);  // No calls until ref goes out of scope
            }
            REQUIRE(changed == 2);
            v.getRef() = getValue<TestType>();
            REQUIRE(changed == 3);
            {
                // Setting the
                auto ref = v.getRef();
                ref = getDefaultValue<TestType>();
                ref = getValue<TestType>();
                ref = getDefaultValue<TestType>();
                ref = getValue<TestType>();
            }
            REQUIRE(changed == 4);
            REQUIRE(compare(v.get(), getValue<TestType>()));
            if constexpr (requires { v.get().getX(); }) {
                v.getRef()->setX(55);
                REQUIRE(changed == 5);
                v.getRef()->getX();
                REQUIRE(changed == 6);  // Still counts as an update
                std::as_const(v).getRef()->getX();
                REQUIRE(changed == 6);  // no call
                v.getCRef()->getX();
                REQUIRE(changed == 6);  // no call
            }
        }
    }
    SECTION("Default compare") {
        if constexpr (requires { std::equal_to<> {}(std::declval<TestType>(), std::declval<TestType>()); }) {
            using Observer = ut::ChangeObserver<TestType, ut::ChangeObserverKeepOldCopy::No, std::equal_to<>>;
            SECTION("Default ctor & update") {
                int changed = 0;
                Observer default_ctor;
                default_ctor.onChange([&](auto &&...) { changed++; });
                default_ctor.onChange([&](TestType const &v) {
                    if (changed != 1) {
                        FAIL("This should never happen: v = " << toString(v));
                    }
                    REQUIRE(compare(v, getValue<TestType>()));
                    changed++;
                });
                default_ctor.update(getValue<TestType>());
                REQUIRE(changed == 2);
                default_ctor.update(getValue<TestType>());
                REQUIRE(changed == 2);
            }
            SECTION("Value ctor & Assignment") {
                int changed = 0;
                Observer value_ctor {getValue<TestType>()};
                value_ctor.onChange([&](auto &&...) { changed++; });
                Observer default_val;
                Observer similar_val {getSimilarValue<TestType>()};
                value_ctor = default_val;
                value_ctor = Observer {getValue<TestType>()};
                value_ctor = Observer {getValue<TestType>()};
                REQUIRE(changed == 2);
            }
            SECTION("getRef & Proxy") {
                int changed = 0;
                Observer v;
                v.onChange([&](auto &&...) { changed++; });
                v.getRef() = getValue<TestType>();
                REQUIRE(changed == 1);
                {
                    auto ref = v.getRef();
                    ref = getDefaultValue<TestType>();
                    REQUIRE(changed == 1);  // No calls until ref goes out of scope
                }
                REQUIRE(changed == 2);
                v.getRef() = getValue<TestType>();
                REQUIRE(changed == 3);
                {
                    // Setting the
                    auto ref = v.getRef();
                    ref = getDefaultValue<TestType>();
                    ref = getValue<TestType>();
                    ref = getDefaultValue<TestType>();
                    ref = getValue<TestType>();
                }
                REQUIRE(changed == 4);  // When reference doesn't hold the copy, the callback still happens
                REQUIRE(compare(v.get(), getValue<TestType>()));
                if constexpr (requires { v.get().getX(); }) {
                    v.getRef()->setX(55);
                    REQUIRE(changed == 5);
                    std::as_const(v).getRef()->getX();
                    REQUIRE(changed == 5);  // no call
                    v.getCRef()->getX();
                    REQUIRE(changed == 5);  // no call
                }
            }
        }
    }
    SECTION("Custom compare") {
        using Observer = ut::ChangeObserver<TestType, ut::ChangeObserverKeepOldCopy::No, CustomCompare<TestType>>;
        SECTION("Default ctor & update") {
            int changed = 0;
            Observer default_ctor;
            default_ctor.onChange([&](auto &&...) { changed++; });
            default_ctor.onChange([&](TestType const &v) {  //
                switch (changed) {
                    case 1: {
                        REQUIRE(compare(v, getValue<TestType>()));
                        break;
                    }
                    case 3: {
                        REQUIRE(compare(v, getDefaultValue<TestType>()));
                        break;
                    }
                    default: FAIL("This should never happen: v = " << toString(v));
                }
                changed++;
            });
            default_ctor.update(getValue<TestType>());
            REQUIRE(changed == 2);
            default_ctor.update(getSimilarValue<TestType>());  // This replaces the value, but does not
                                                               // trigger the callbacks
            REQUIRE(changed == 2);
            default_ctor.update(getDefaultValue<TestType>());
            REQUIRE(changed == 4);
        }
        SECTION("Value ctor & Assignment") {
            int changed = 0;
            Observer value_ctor {getValue<TestType>()};
            value_ctor.onChange([&](auto &&...) { changed++; });
            Observer default_val;
            Observer similar_val {getSimilarValue<TestType>()};
            value_ctor = default_val;
            value_ctor = std::move(similar_val);
            value_ctor = Observer {getValue<TestType>()};
            REQUIRE(changed == 2);
        }
        SECTION("getRef & Proxy") {
            int changed = 0;
            Observer v;
            v.onChange([&](auto &&...) { changed++; });
            v.getRef() = getValue<TestType>();
            REQUIRE(changed == 1);
            {
                auto ref = v.getRef();
                ref = getDefaultValue<TestType>();
                REQUIRE(changed == 1);  // No calls until ref goes out of scope
            }
            REQUIRE(changed == 2);
            v.getRef() = getValue<TestType>();
            REQUIRE(changed == 3);
            {
                // Setting the
                auto ref = v.getRef();
                ref = getDefaultValue<TestType>();
                ref = getSimilarValue<TestType>();
                ref = getDefaultValue<TestType>();
                ref = getSimilarValue<TestType>();
            }
            REQUIRE(changed == 4);  // When reference doesn't hold the copy, the callback still happens
            REQUIRE(compare(v.get(), getSimilarValue<TestType>()));
            if constexpr (requires { v.get().getX(); }) {
                v.getRef()->setX(55);
                REQUIRE(changed == 5);
                std::as_const(v).getRef()->getX();
                REQUIRE(changed == 5);  // no call
                v.getCRef()->getX();
                REQUIRE(changed == 5);  // no call
            }
        }
    }
}
