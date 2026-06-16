#include <catch.hpp>

#include <algorithm>
#include <iterator>
#include <limits>
#include <memory>
#include <numeric>
#include <ranges>

#ifdef assert
#    undef assert
#endif
#define assert REQUIRE
#include <ut/ptr_containers/ownptrvec.hpp>

using namespace ut;

TEST_CASE("OwnPtrVec constructors", "[ownptrvec]") {

    SECTION("Empty constructor") {
        OwnPtrVec<int> v;
        REQUIRE(v.size() == 0);
        REQUIRE(v.capacity() == 0);
        REQUIRE(v.data() == nullptr);
    }
    SECTION("Reserve constructor") {
        auto v = OwnPtrVec<int>::fromReserve(10);
        REQUIRE(v.size() == 0);
        REQUIRE(v.capacity() == 10);
        REQUIRE(v.data() != nullptr);
    }

    SECTION("From list constructor") {
        auto v = OwnPtrVec<int>::make(1, std::make_unique<int>(2), 3);
        REQUIRE(v.size() == 3);
        REQUIRE(v.capacity() == 3);
        REQUIRE(v.data() != nullptr);
        REQUIRE(*v[0] == 1);
        REQUIRE(*v[1] == 2);
        REQUIRE(*v[2] == 3);
    }

    SECTION("Move constructor") {
        auto v1 = OwnPtrVec<int>::fromReserve(10);
        auto v2 = std::move(v1);
        REQUIRE(v1.size() == 0);
        REQUIRE(v1.capacity() == 0);
        REQUIRE(v1.data() == nullptr);

        REQUIRE(v2.size() == 0);
        REQUIRE(v2.capacity() == 10);
        REQUIRE(v2.data() != nullptr);
    }
}

TEST_CASE("OwnPtrVec element access", "[ownptrvec]") {
    auto v = OwnPtrVec<int>::make(23, 42);
    REQUIRE(v.size() == 2);
    OwnPtrVec<int> const &cv = v;

    SECTION("non-const operator[]") {
        REQUIRE(*v[0] == 23);
        *v[1] = 10;
        REQUIRE(*v[1] == 10);
    }
    SECTION("const operator[]") {
        REQUIRE(*cv[0] == 23);
    }

    SECTION("front") {
        REQUIRE(*v.front() == 23);
        REQUIRE(*v.front() == *v[0]);
        *v.front() = 10;
        REQUIRE(*v.front() == 10);
    }
    SECTION("const front") {
        REQUIRE(*cv.front() == 23);
        REQUIRE(*cv.front() == *v[0]);
    }
    SECTION("back") {
        REQUIRE(*v.back() == 42);
        REQUIRE(*v.back() == *v[1]);
        *v.back() = 10;
        REQUIRE(*v.back() == 10);
    }
    SECTION("const back") {
        REQUIRE(*cv.back() == 42);
        REQUIRE(*cv.back() == *v[1]);
    }

    SECTION("data") {
        int **ptr = v.data();
        REQUIRE(*ptr[0] == 23);
        *ptr[1] = 3;
        REQUIRE(*v[1] == 3);
    }
    SECTION("const data") {
        REQUIRE(*cv.data()[0] == 23);
    }
    SECTION("release") {
        auto size = v.size();
        auto ptr = v.release();
        REQUIRE(v.size() == 0);
        REQUIRE(v.capacity() == 0);
        REQUIRE(v.data() == nullptr);
        for (std::size_t i = 0; i < size; i++)
            delete ptr[i];
        delete[] ptr;
        SUCCEED("no leak and no double delete");
    }
    SECTION("release_back") {
        auto old_cap = v.capacity();
        auto ptr = v.release_back();
        REQUIRE(v.size() == 1);
        REQUIRE(v.capacity() == old_cap);
        SUCCEED("no leak and no double delete");
    }
}

TEST_CASE("OwnPtrVec iterators", "[ownptrvec]") {
    OwnPtrVec<int> empty;
    OwnPtrVec<int> const cempty;
    auto v = OwnPtrVec<int>::make(1, 2, 3, 4, 5);
    OwnPtrVec<int> const &cv = v;

    SECTION("begin") {
        REQUIRE(*v.begin() == v.front());
        REQUIRE(*cv.begin() == cv.front());
        REQUIRE(*v.cbegin() == v.front());
        REQUIRE(*cv.cbegin() == cv.front());
        REQUIRE(*v.rbegin() == v.back());
        REQUIRE(*cv.rbegin() == cv.back());
        REQUIRE(*v.crbegin() == v.back());
        REQUIRE(*cv.crbegin() == cv.back());
    }
    SECTION("end") {
        REQUIRE(*(v.end() - 1) == v.back());
        REQUIRE(*(cv.end() - 1) == cv.back());
        REQUIRE(*(v.cend() - 1) == v.back());
        REQUIRE(*(cv.cend() - 1) == cv.back());
        REQUIRE(*(v.rend() - 1) == v.front());
        REQUIRE(*(cv.rend() - 1) == cv.front());
        REQUIRE(*(v.crend() - 1) == v.front());
        REQUIRE(*(cv.crend() - 1) == cv.front());
    }


    SECTION("empty") {
        REQUIRE(empty.begin() == empty.end());
        REQUIRE(cempty.begin() == cempty.end());
        REQUIRE(empty.cbegin() == empty.cend());
        REQUIRE(cempty.cbegin() == cempty.cend());
        REQUIRE(empty.rbegin() == empty.rend());
        REQUIRE(cempty.rbegin() == cempty.rend());
        REQUIRE(empty.crbegin() == empty.crend());
        REQUIRE(cempty.crbegin() == cempty.crend());
    }

    SECTION("iterator for loop") {
        int i = 1;
        for (auto it = v.begin(); it != v.end(); ++it, i++)
            REQUIRE(**it == i);
        REQUIRE(std::size_t(i) == v.size() + 1);

        i = 1;
        for (auto it = v.cbegin(); it != v.cend(); ++it, i++)
            REQUIRE(**it == i);
        REQUIRE(std::size_t(i) == v.size() + 1);

        i = 1;
        for (auto it = cv.begin(); it != cv.end(); ++it, i++)
            REQUIRE(**it == i);
        REQUIRE(std::size_t(i) == cv.size() + 1);

        i = 1;
        for (auto it = cv.cbegin(); it != cv.cend(); ++it, i++)
            REQUIRE(**it == i);
        REQUIRE(std::size_t(i) == cv.size() + 1);


        i = int(v.size());
        for (auto it = v.rbegin(); it != v.rend(); ++it, i--)
            REQUIRE(**it == i);
        REQUIRE(i == 0);

        i = int(v.size());
        for (auto it = v.crbegin(); it != v.crend(); ++it, i--)
            REQUIRE(**it == i);
        REQUIRE(i == 0);

        i = int(cv.size());
        for (auto it = cv.rbegin(); it != cv.rend(); ++it, i--)
            REQUIRE(**it == i);
        REQUIRE(i == 0);

        i = int(cv.size());
        for (auto it = cv.crbegin(); it != cv.crend(); ++it, i--)
            REQUIRE(**it == i);
        REQUIRE(i == 0);
    }
    SECTION("range based for loop") {
        int i = 1;
        for (auto elem : v)
            REQUIRE(*elem == i++);
        REQUIRE(std::size_t(i) == v.size() + 1);

        i = 1;
        for (int *elem : cv)
            REQUIRE(*elem == i++);
        REQUIRE(std::size_t(i) == cv.size() + 1);
    }
}

TEST_CASE("OwnPtrVec capacity", "[ownptrvec]") {
    OwnPtrVec<int> empty;
    auto reserved = OwnPtrVec<int>::fromReserve(10);
    auto v = OwnPtrVec<int>::make(1, 2, 3, 4);

    SECTION("empty") {
        REQUIRE(empty.empty());
        REQUIRE(reserved.empty());
        REQUIRE_FALSE(v.empty());
    }

    SECTION("size") {
        REQUIRE(empty.size() == 0);
        REQUIRE(reserved.size() == 0);
        REQUIRE(v.size() == 4);
    }

    SECTION("max_size") {
        REQUIRE(empty.max_size() == std::numeric_limits<std::size_t>::max());
        REQUIRE(empty.max_size() == v.max_size());
    }

    SECTION("reserve") {
        REQUIRE(empty.capacity() == 0);
        empty.reserve(10);
        REQUIRE(empty.capacity() == 10);
        empty.reserve(1);
        REQUIRE(empty.capacity() == 10);
    }

    SECTION("capacity") {
        REQUIRE(reserved.capacity() == 10);
        REQUIRE(v.capacity() >= v.size());
        REQUIRE(empty.capacity() == 0);
    }

    SECTION("shrink_to_fit") {
        empty.shrink_to_fit();
        reserved.shrink_to_fit();
        v.shrink_to_fit();
        REQUIRE(empty.capacity() == 0);
        REQUIRE(reserved.capacity() == 0);
        REQUIRE(v.capacity() == v.size());
    }
}

TEST_CASE("OwnPtrVec modifiers", "[ownptrvec]") {
    struct S {
        S(int, int, int) { }
    };

    OwnPtrVec<S> empty;
    auto v = OwnPtrVec<S>::make(S {1, 2, 3});
    auto iv = OwnPtrVec<int>::make(1, 2);

    REQUIRE(empty.size() == 0);
    REQUIRE(v.size() == 1);
    REQUIRE(iv.size() == 2);

    SECTION("clear") {
        empty.clear();
        v.clear();
        REQUIRE(empty.size() == 0);
        REQUIRE(v.size() == 0);
    }
    SECTION("push_back") {
        auto ptr = std::make_unique<S>(1, 2, 3);
        empty.push_back(std::move(ptr));
        empty.push_back(std::make_unique<S>(1, 2, 3));
        empty.push_back(S {1, 2, 3});
        S s {1, 3, 4};
        empty.push_back(s);
        REQUIRE(empty.size() == 4);
        REQUIRE(empty.capacity() >= 4);
    }
    SECTION("emplace_back") {
        empty.emplace_back(1, 2, 3);
        empty.emplace_back(S {1, 2, 3});
        S s {1, 2, 3};
        empty.emplace_back(s);
        REQUIRE(empty.size() == 3);
    }
    SECTION("pop_back") {
        v.pop_back();
        REQUIRE(v.size() == 0);
    }
    SECTION("insert") {
        iv.insert(iv.begin(), 42);
        REQUIRE(**iv.begin() == 42);
        REQUIRE(iv.size() == 3);
        REQUIRE(*iv[0] == 42);
        REQUIRE(*iv[1] == 1);
        REQUIRE(*iv[2] == 2);

        auto ptr = std::make_unique<int>(100);
        iv.insert(iv.begin() + 1, std::move(ptr));
        REQUIRE(**(iv.begin() + 1) == 100);
        REQUIRE(iv.size() == 4);
        REQUIRE(*iv[0] == 42);
        REQUIRE(*iv[1] == 100);
        REQUIRE(*iv[2] == 1);
        REQUIRE(*iv[3] == 2);

        iv.insert(iv.end(), std::make_unique<int>(999));
        REQUIRE(**(iv.end() - 1) == 999);
        REQUIRE(iv.size() == 5);
        REQUIRE(*iv[0] == 42);
        REQUIRE(*iv[1] == 100);
        REQUIRE(*iv[2] == 1);
        REQUIRE(*iv[3] == 2);
        REQUIRE(*iv[4] == 999);
    }
    SECTION("erase") {
        iv.erase(iv.begin());
        REQUIRE(iv.size() == 1);
        REQUIRE(*iv.front() == 2);
        iv.push_back(10);
        iv.push_back(11);
        auto it = iv.erase(iv.end() - 1);
        REQUIRE(*iv[0] == 2);
        REQUIRE(*iv[1] == 10);
        REQUIRE(iv.size() == 2);
        REQUIRE(it == iv.end());

        iv.push_back(11);
        iv.push_back(12);
        iv.push_back(13);
        it = iv.erase(iv.begin() + 1, iv.begin() + 3);
        REQUIRE(*iv[0] == 2);
        REQUIRE(*iv[1] == 12);
        REQUIRE(*iv[2] == 13);
        REQUIRE(iv.size() == 3);
        REQUIRE(iv.begin() + 1 == it);
        iv.erase(iv.begin(), iv.end());
        REQUIRE(iv.size() == 0);
    }
    SECTION("emplace") {
        empty.emplace(empty.begin(), 1, 2, 3);
        REQUIRE(empty.size() == 1);
        iv.emplace(iv.begin() + 1, 77);
        REQUIRE(iv.size() == 3);
        REQUIRE(*iv[1] == 77);
    }
    SECTION("swap") {
        auto iv2 = OwnPtrVec<int>::make(9999, 8888, 7777, 6666);
        REQUIRE(iv2.size() == 4);

        iv2.swap(iv);

        REQUIRE(iv2.size() == 2);
        REQUIRE(*iv2[0] == 1);
        REQUIRE(*iv2[1] == 2);

        REQUIRE(iv.size() == 4);
        REQUIRE(*iv[0] == 9999);
        REQUIRE(*iv[1] == 8888);
        REQUIRE(*iv[2] == 7777);
        REQUIRE(*iv[3] == 6666);
    }
}

TEST_CASE("OwnPtrVec comparison", "[ownptrvec]") {
    auto same1 = OwnPtrVec<int>::make(1, 2, 3, 4);
    auto same2 = OwnPtrVec<int>::make(1, 2, 3, 4);
    auto diff1 = OwnPtrVec<int>::make(100, 200, 300, 400);
    auto diff2 = OwnPtrVec<int>::make(100, 200);
    OwnPtrVec<int> empty1;
    OwnPtrVec<int> empty2;

    REQUIRE(same1 == same1);
    REQUIRE(empty1 == empty2);
    REQUIRE(same1 == same2);
    REQUIRE(same2 == same1);
    REQUIRE(same1 != diff1);
    REQUIRE(same1 != diff2);
    REQUIRE(same1 != empty1);

    REQUIRE_FALSE(same1 != same1);
    REQUIRE_FALSE(empty1 != empty2);
    REQUIRE_FALSE(same1 != same2);
    REQUIRE_FALSE(same2 != same1);
    REQUIRE_FALSE(same1 == diff1);
    REQUIRE_FALSE(same1 == diff2);
    REQUIRE_FALSE(same1 == empty1);
}

TEST_CASE("OwnPtrVec stdlib integration", "[ownptrvec]") {
    OwnPtrVec<int> v;
    v.push_back(1);

    SECTION("data") {
        REQUIRE(std::data(v) == v.data());
    }
    SECTION("begin") {
        REQUIRE(std::begin(v) == v.begin());
        REQUIRE(std::rbegin(v) == v.rbegin());
        REQUIRE(std::cbegin(v) == v.cbegin());
        REQUIRE(std::crbegin(v) == v.crbegin());
    }
    SECTION("end") {
        REQUIRE(std::end(v) == v.end());
        REQUIRE(std::rend(v) == v.rend());
        REQUIRE(std::cend(v) == v.cend());
        REQUIRE(std::crend(v) == v.crend());
    }
    SECTION("size") {
        REQUIRE(std::size(v) == v.size());
    }
    SECTION("swap") {
        OwnPtrVec<int> std1;
        std1.push_back(10);
        std1.push_back(11);
        OwnPtrVec<int> std2;
        std1.push_back(9);

        OwnPtrVec<int> sw1;
        sw1.push_back(10);
        sw1.push_back(11);
        OwnPtrVec<int> sw2;
        sw1.push_back(9);

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
    SECTION("algorithm") {
        OwnPtrVec<int> ac;
        ac.push_back(10);
        ac.push_back(10);
        ac.push_back(10);
        ac.push_back(10);
        auto res = std::accumulate(std::begin(ac), std::end(ac), 0, [](int acc, auto val) { return acc + *val; });
        REQUIRE(res == 40);
    }

    SECTION("stack") {
        // this is a std::stack
        OwnPtrStack<int> s;
        REQUIRE(s.size() == 0);
        s.push(1);
        s.emplace(2);
        REQUIRE(s.size() == 2);
        REQUIRE(*s.top() == 2);
        s.pop();
        REQUIRE(s.size() == 1);
        REQUIRE(*s.top() == 1);
        s.pop();
        REQUIRE(s.size() == 0);
    }
}

TEST_CASE("OwnPtrVec overloaded constructors", "[ownptrvec]") {
    SECTION("No copy") {
        struct NoCpy {
            NoCpy() = default;
            NoCpy(NoCpy const &) = delete;
            NoCpy &operator=(NoCpy const &) = delete;
            NoCpy(NoCpy &&) = default;
            [[maybe_unused]] NoCpy &operator=(NoCpy &&) = default;
            [[maybe_unused]] ~NoCpy() = default;
        };

        OwnPtrVec<NoCpy> v;
        v.push_back(NoCpy {});
        NoCpy nc;
        v.push_back(std::move(nc));
        v.push_back(std::make_unique<NoCpy>());
        SUCCEED("no copy");
    }
    SECTION("No move") {
        struct NoMove {
            NoMove() = default;
            NoMove(NoMove const &) = default;
            NoMove &operator=(NoMove const &) = default;
            NoMove(NoMove &&) = delete;
            NoMove &operator=(NoMove &&) = delete;
            [[maybe_unused]] ~NoMove() = default;
        };

        OwnPtrVec<NoMove> v;
        NoMove nm;
        v.push_back(nm);
        v.push_back(std::make_unique<NoMove>());
        SUCCEED("no move");
    }
    SECTION("No move no copy") {
        struct NoCpyNoMove {
            [[maybe_unused]] NoCpyNoMove() = default;
            NoCpyNoMove(NoCpyNoMove const &) = delete;
            NoCpyNoMove &operator=(NoCpyNoMove const &) = delete;
            NoCpyNoMove(NoCpyNoMove &&) = delete;
            NoCpyNoMove &operator=(NoCpyNoMove &&) = delete;
            [[maybe_unused]] ~NoCpyNoMove() = default;
        };

        OwnPtrVec<NoCpyNoMove> v;
        v.push_back(std::make_unique<NoCpyNoMove>());
        SUCCEED("no move no copy");
    }
    SECTION("unique_ptr") {
        OwnPtrVec<std::unique_ptr<int>> v;
        v.push_back(std::make_unique<std::unique_ptr<int>>(nullptr));
        SUCCEED("unique_ptr");
    }
}

TEST_CASE("OwnPtrVec inheritance", "[ownptrvec]") {
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

    SECTION("push_back") {
        OwnPtrVec<Base> v;
        v.push_back(Base {});
        v.push_back(Derived {});
        REQUIRE(v.size() == 2);
        REQUIRE(v[0]->isBase() == true);
        REQUIRE(v[1]->isBase() == false);
    }
    SECTION("make constructor") {
        auto v = OwnPtrVec<Base>::make(Base {}, Derived {}, std::make_unique<Derived>());
        REQUIRE(v.size() == 3);
        REQUIRE(v[0]->isBase() == true);
        REQUIRE(v[1]->isBase() == false);
        REQUIRE(v[2]->isBase() == false);
    }
}

TEST_CASE("OwnPtrVec proxy reference", "[ownptrvec]") {
    struct Tracker {
        int *destroyed;
        int val;

        Tracker(int *d, int v)
                : destroyed(d)
                , val(v) { }

        ~Tracker() {
            ++*destroyed;
        }
    };

    SECTION("assign from unique_ptr deletes the old element") {
        int destroyed = 0;
        auto v = OwnPtrVec<Tracker>::make(Tracker {&destroyed, 1});
        REQUIRE(v[0]->val == 1);
        destroyed = 0;  // discount the temporaries make() copied from

        *v.begin() = std::make_unique<Tracker>(&destroyed, 2);
        REQUIRE(destroyed == 1);
        REQUIRE(v[0]->val == 2);
        REQUIRE(v.size() == 1);
    }

    SECTION("proxy assignment transfers ownership between slots") {
        int destroyed = 0;
        auto v = OwnPtrVec<Tracker>::make(Tracker {&destroyed, 1}, Tracker {&destroyed, 2});
        destroyed = 0;  // discount the temporaries make() copied from

        *v.begin() = *(v.begin() + 1);
        REQUIRE(destroyed == 1);
        REQUIRE(v[0]->val == 2);
        REQUIRE(!v[1]);
    }

    SECTION("swap exchanges the two slots") {
        auto v = OwnPtrVec<int>::make(1, 2);
        swap(*v.begin(), *(v.begin() + 1));
        REQUIRE(*v[0] == 2);
        REQUIRE(*v[1] == 1);
    }

    SECTION("proxy compares by pointer identity") {
        auto v = OwnPtrVec<int>::make(1, 2);
        REQUIRE(*v.begin() == v.front());
        REQUIRE(v.front() == v[0]);
        REQUIRE_FALSE(v[0] == v[1]);
    }
}

TEST_CASE("OwnPtrVec iterator algorithms", "[ownptrvec]") {
    SECTION("std::sort") {
        auto v = OwnPtrVec<int>::make(3, 1, 4, 1, 5, 9, 2, 6);
        std::sort(v.begin(), v.end(), [](auto const &a, auto const &b) { return *a < *b; });

        int prev = std::numeric_limits<int>::min();
        for (auto e : v) {
            REQUIRE(*e >= prev);
            prev = *e;
        }
        REQUIRE(v.size() == 8);
    }

    SECTION("std::transform with make_move_iterator (issue #1 example)") {
        auto v = OwnPtrVec<int>::make(1, 2, 3);
        std::transform(
            std::make_move_iterator(v.begin()),
            std::make_move_iterator(v.end()),
            v.begin(),
            [](auto node) -> std::unique_ptr<int> {
                if (*node % 2 == 0) return node;
                return std::make_unique<int>(*node * 10);
            });
        REQUIRE(*v[0] == 10);
        REQUIRE(*v[1] == 2);
        REQUIRE(*v[2] == 30);
    }

    SECTION("iter_move extracts ownership and empties the slot") {
        auto v = OwnPtrVec<int>::make(7);
        std::unique_ptr<int> owned = std::ranges::iter_move(v.begin());
        REQUIRE(owned);
        REQUIRE(*owned == 7);
        REQUIRE(!*v.begin());
    }
}

TEST_CASE("OwnPtrVec iterator concepts", "[ownptrvec]") {
    using It = OwnPtrVec<int>::iterator;

    STATIC_REQUIRE(std::same_as<std::iter_value_t<It>, std::unique_ptr<int>>);
    STATIC_REQUIRE(std::same_as<std::iter_reference_t<It>, detail::OwnPtrRef<int>>);

    STATIC_REQUIRE(std::random_access_iterator<It>);
    STATIC_REQUIRE(std::sortable<It, decltype([](auto const &a, auto const &b) { return *a < *b; })>);
    STATIC_REQUIRE(std::ranges::random_access_range<OwnPtrVec<int>>);
}

TEST_CASE("OwnPtrVec ranges algorithms", "[ownptrvec]") {
    SECTION("ranges::sort with projection") {
        auto v = OwnPtrVec<int>::make(3, 1, 4, 1, 5, 9, 2, 6);
        std::ranges::sort(v, {}, [](auto const &r) noexcept { return *r; });
        REQUIRE(std::ranges::is_sorted(v, {}, [](auto const &r) noexcept { return *r; }));
        REQUIRE(*v.front() == 1);
        REQUIRE(*v.back() == 9);
    }

    SECTION("ranges::for_each mutates in place") {
        auto v = OwnPtrVec<int>::make(1, 2, 3);
        std::ranges::for_each(v, [](auto const &r) noexcept { *r *= 10; });
        REQUIRE(*v[0] == 10);
        REQUIRE(*v[1] == 20);
        REQUIRE(*v[2] == 30);
    }

    SECTION("ranges::count_if / find_if with projection") {
        auto v = OwnPtrVec<int>::make(1, 2, 3, 4, 5, 6);
        REQUIRE(std::ranges::count_if(v, [](int x) noexcept { return x % 2 == 0; }, [](auto const &r) noexcept { return *r; }) == 3);

        auto it = std::ranges::find_if(v, [](auto const &r) noexcept { return *r == 4; });
        REQUIRE(it != v.end());
        REQUIRE(**it == 4);
    }

    SECTION("ranges::max_element with projection") {
        auto v = OwnPtrVec<int>::make(3, 9, 2, 7);
        REQUIRE(**std::ranges::max_element(v, {}, [](auto const &r) noexcept { return *r; }) == 9);
    }

    SECTION("ranges::transform in place") {
        auto v = OwnPtrVec<int>::make(1, 2, 3);
        std::ranges::transform(v, v.begin(), [](auto const &r) { return std::make_unique<int>(*r + 100); });
        REQUIRE(*v[0] == 101);
        REQUIRE(*v[1] == 102);
        REQUIRE(*v[2] == 103);
    }
}

TEST_CASE("OwnPtrVec proxy polymorphism", "[ownptrvec]") {
    struct Base {
        virtual int id() const {
            return 0;
        }

        virtual ~Base() = default;
    };

    struct Derived : Base {
        int id() const override {
            return 1;
        }
    };

    SECTION("assign a derived object through the proxy (no slicing)") {
        auto v = OwnPtrVec<Base>::make(Base {}, Base {});
        *v.begin() = std::make_unique<Derived>();
        REQUIRE((*v.begin())->id() == 1);
        REQUIRE(v[1]->id() == 0);
    }

    SECTION("iter_move yields a unique_ptr<Base> that still dispatches to Derived") {
        auto v = OwnPtrVec<Base>::make(Base {});
        *v.begin() = std::make_unique<Derived>();
        std::unique_ptr<Base> b = std::ranges::iter_move(v.begin());
        REQUIRE(b->id() == 1);
    }

    SECTION("sort polymorphic elements") {
        auto v = OwnPtrVec<Base>::make(Derived {}, Base {});  // ids: 1, 0
        std::sort(v.begin(), v.end(), [](auto const &a, auto const &b) { return a->id() < b->id(); });
        REQUIRE(v[0]->id() == 0);
        REQUIRE(v[1]->id() == 1);
    }
}

TEST_CASE("OwnPtrVec incomplete type", "[ownptrvec]") {
    SECTION("definition available in same translation unit") {
        // Forward-declared here so the new-expression below references an
        // existing name; MSVC rejects introducing a type inside new (C2462).
        struct S;

        // Allocating on the heap to control lifetime
        auto *v = new OwnPtrVec<S>();


        bool deleted = false;

        struct S {
            bool *m_i;

            S(bool *i)
                    : m_i(i) { }

            ~S() {
                *m_i = true;
            }
        };

        v->emplace_back(&deleted);
        delete v;
        REQUIRE(deleted);
    }
}

TEST_CASE("OwnPtrVec static assertions", "[ownptrvec]") {

    SECTION("IsDerivedFromContainerBaseV") {
        STATIC_REQUIRE(IsDerivedFromContainerBaseV<OwnPtrVec<int>>);
        STATIC_REQUIRE(IsDerivedFromContainerBaseV<OwnPtrVec<struct S>>);
        STATIC_REQUIRE(IsDerivedFromContainerBaseV<PtrVecView<int>>);
        STATIC_REQUIRE(IsDerivedFromContainerBaseV<PtrVecView<struct S>>);

        STATIC_REQUIRE(!IsDerivedFromContainerBaseV<int>);
        STATIC_REQUIRE(!IsDerivedFromContainerBaseV<struct S>);
    }


    SECTION("IsComparableContainerBaseV") {
        STATIC_REQUIRE(IsComparableContainerBaseV<OwnPtrVec<int>, OwnPtrVec<int>>);
        STATIC_REQUIRE(IsComparableContainerBaseV<OwnPtrVec<struct S>, OwnPtrVec<struct S>>);
        STATIC_REQUIRE(!IsComparableContainerBaseV<OwnPtrVec<struct S>, OwnPtrVec<int>>);

        STATIC_REQUIRE(IsComparableContainerBaseV<PtrVecView<int>, OwnPtrVec<int>>);
        STATIC_REQUIRE(IsComparableContainerBaseV<PtrVecView<struct S>, OwnPtrVec<struct S>>);
        STATIC_REQUIRE(!IsComparableContainerBaseV<PtrVecView<struct S>, OwnPtrVec<int>>);

        STATIC_REQUIRE(IsComparableContainerBaseV<OwnPtrVec<int>, PtrVecView<int>>);
        STATIC_REQUIRE(IsComparableContainerBaseV<OwnPtrVec<struct S>, PtrVecView<struct S>>);
        STATIC_REQUIRE(!IsComparableContainerBaseV<OwnPtrVec<struct S>, PtrVecView<int>>);

        STATIC_REQUIRE(IsComparableContainerBaseV<PtrVecView<int>, PtrVecView<int>>);
        STATIC_REQUIRE(IsComparableContainerBaseV<PtrVecView<struct S>, PtrVecView<struct S>>);
        STATIC_REQUIRE(!IsComparableContainerBaseV<PtrVecView<struct S>, PtrVecView<int>>);
    }
}
