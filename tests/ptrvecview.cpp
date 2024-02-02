#include <catch.hpp>

#include <limits>
#include <memory>
#include <numeric>
#include <stack>

#ifdef assert
#    undef assert
#endif
#define assert REQUIRE
#include <ut/ptr_containers/ownptrvec.hpp>
#include <ut/ptr_containers/ptrvecview.hpp>
#undef assert

using namespace ut;

TEST_CASE("PtrVecView constructors", "[ptrvecview]") {
    auto v = OwnPtrVec<int>::make(1, 2, 3, 4);

    SECTION("Empty constructor") {
        PtrVecView<int> view;
        REQUIRE(view.size() == 0);
        REQUIRE(view.data() == nullptr);
    }
    SECTION("Two iterator constructor") {
        auto view1 = PtrVecView<int> {v.cbegin(), v.cend()};
        REQUIRE(view1.size() == 4);
        REQUIRE(*view1[0] == 1);
        REQUIRE(*view1[1] == 2);
        REQUIRE(*view1[2] == 3);
        REQUIRE(*view1[3] == 4);

        auto view2 = PtrVecView<int> {v.cbegin() + 1, v.cend() - 1};
        REQUIRE(view2.size() == 2);
        REQUIRE(*view2[0] == 2);
        REQUIRE(*view2[1] == 3);
    }

    SECTION("Iterator and count constructor") {
        auto view1 = PtrVecView<int> {v.cbegin(), v.size()};
        REQUIRE(view1.size() == 4);
        REQUIRE(*view1[0] == 1);
        REQUIRE(*view1[1] == 2);
        REQUIRE(*view1[2] == 3);
        REQUIRE(*view1[3] == 4);

        auto view2 = PtrVecView<int> {v.cbegin() + 1, 2};
        REQUIRE(view2.size() == 2);
        REQUIRE(*view2[0] == 2);
        REQUIRE(*view2[1] == 3);
    }

    SECTION("View member function") {
        auto view1 = v.view();
        REQUIRE(view1.size() == 4);
        REQUIRE(*view1[0] == 1);
        REQUIRE(*view1[1] == 2);
        REQUIRE(*view1[2] == 3);
        REQUIRE(*view1[3] == 4);

        auto view2 = v.view(1);
        REQUIRE(view2.size() == 3);
        REQUIRE(*view2[0] == 2);
        REQUIRE(*view2[1] == 3);
        REQUIRE(*view2[2] == 4);

        auto view3 = v.view(1, v.size() - 1);
        REQUIRE(view3.size() == 2);
        REQUIRE(*view3[0] == 2);
        REQUIRE(*view3[1] == 3);

        OwnPtrVec<int> empty;
        auto empty_view = empty.view();
        REQUIRE(empty_view.size() == 0);
        REQUIRE(empty_view.data() == nullptr);
    }

    SECTION("Copy constructor") {
        auto view1 = v.view();
        auto view2 = view1;
        REQUIRE(view1[0] == view2[0]);
        REQUIRE(view1[1] == view2[1]);
        REQUIRE(view1[2] == view2[2]);
        REQUIRE(view1[3] == view2[3]);
        REQUIRE(view1.size() == view2.size());
        REQUIRE(view1.data() == view2.data());
    }

    SECTION("Move constructor") {
        auto view1 = v.view();
        auto view2 = std::move(view1);
        REQUIRE(view1[0] == view2[0]);
        REQUIRE(view1[1] == view2[1]);
        REQUIRE(view1[2] == view2[2]);
        REQUIRE(view1[3] == view2[3]);
        REQUIRE(view1.size() == view2.size());
        REQUIRE(view1.data() == view2.data());
    }
}

TEST_CASE("PtrVecView element access", "[ptrvecview]") {
    auto v = OwnPtrVec<int>::make(1, 2);
    auto view = v.view();
    REQUIRE(view.size() == 2);

    SECTION("operator[]") {
        REQUIRE(*view[0] == 1);
        REQUIRE(*view[1] == 2);
    }

    SECTION("front") {
        REQUIRE(*view.front() == 1);
        REQUIRE(*view.front() == *view[0]);
    }
    SECTION("back") {
        REQUIRE(*view.back() == 2);
        REQUIRE(*view.back() == *view[1]);
    }

    SECTION("data") {
        int const *const *ptr = view.data();
        REQUIRE(*ptr[0] == 1);
        REQUIRE(*view[1] == 2);
    }
}

TEST_CASE("PtrVecView iterators", "[ptrvecview]") {
    auto v = OwnPtrVec<int>::make(1, 2, 3, 4, 5);
    auto view = v.view();
    PtrVecView<int> empty;

    SECTION("begin") {
        REQUIRE(*view.begin() == view.front());
        REQUIRE(*view.cbegin() == view.front());
        REQUIRE(*view.rbegin() == view.back());
        REQUIRE(*view.crbegin() == view.back());
    }
    SECTION("end") {
        REQUIRE(*(view.end() - 1) == view.back());
        REQUIRE(*(view.cend() - 1) == view.back());
        REQUIRE(*(view.rend() - 1) == view.front());
        REQUIRE(*(view.crend() - 1) == view.front());
    }


    SECTION("empty") {
        REQUIRE(empty.begin() == empty.end());
        REQUIRE(empty.cbegin() == empty.cend());
        REQUIRE(empty.rbegin() == empty.rend());
        REQUIRE(empty.crbegin() == empty.crend());
    }

    SECTION("iterator for loop") {
        int i = 1;
        for (auto it = view.begin(); it != view.end(); ++it, i++)
            REQUIRE(**it == i);
        REQUIRE(std::size_t(i) == view.size() + 1);

        i = 1;
        for (auto it = view.cbegin(); it != view.cend(); ++it, i++)
            REQUIRE(**it == i);
        REQUIRE(std::size_t(i) == view.size() + 1);


        i = int(view.size());
        for (auto it = view.rbegin(); it != view.rend(); ++it, i--)
            REQUIRE(**it == i);
        REQUIRE(i == 0);

        i = int(view.size());
        for (auto it = view.crbegin(); it != view.crend(); ++it, i--)
            REQUIRE(**it == i);
        REQUIRE(i == 0);
    }
    SECTION("range based for loop") {
        int i = 1;
        for (int const *elem : view)
            REQUIRE(*elem == i++);
        REQUIRE(std::size_t(i) == view.size() + 1);
    }
}

TEST_CASE("PtrVecView capacity", "[ptrvecview]") {
    auto v = OwnPtrVec<int>::make(1, 2, 3, 4);
    PtrVecView<int> empty;
    auto view = v.view();

    SECTION("empty") {
        REQUIRE(empty.empty());
        REQUIRE_FALSE(view.empty());
    }

    SECTION("size") {
        REQUIRE(empty.size() == 0);
        REQUIRE(view.size() == 4);
    }

    SECTION("max_size") {
        REQUIRE(empty.max_size() == std::numeric_limits<std::size_t>::max());
        REQUIRE(empty.max_size() == view.max_size());
    }
}

TEST_CASE("PtrVecView comparison", "[ptrvecview]") {
    auto same1 = OwnPtrVec<int>::make(1, 2, 3, 4);
    auto same2 = OwnPtrVec<int>::make(1, 2, 3, 4);
    auto diff1 = OwnPtrVec<int>::make(100, 200, 300, 400);
    auto diff2 = OwnPtrVec<int>::make(100, 200);
    OwnPtrVec<int> empty1;
    OwnPtrVec<int> empty2;
    auto same1_view = same1.view();
    auto same2_view = same2.view();
    auto diff1_view = diff1.view();
    auto diff2_view = diff2.view();
    auto empty1_view = empty1.view();
    auto empty2_view = empty2.view();

    SECTION("View == View") {
        REQUIRE(same1_view == same1_view);
        REQUIRE(empty1_view == empty2_view);
        REQUIRE(same1_view == same2_view);
        REQUIRE(same2_view == same1_view);
        REQUIRE(same1_view != diff1_view);
        REQUIRE(same1_view != diff2_view);
        REQUIRE(same1_view != empty1_view);

        REQUIRE_FALSE(same1_view != same1_view);
        REQUIRE_FALSE(empty1_view != empty2_view);
        REQUIRE_FALSE(same1_view != same2_view);
        REQUIRE_FALSE(same2_view != same1_view);
        REQUIRE_FALSE(same1_view == diff1_view);
        REQUIRE_FALSE(same1_view == diff2_view);
        REQUIRE_FALSE(same1_view == empty1_view);
    }

    SECTION("Vec == View") {
        REQUIRE(same1 == same1_view);
        REQUIRE(empty1 == empty2_view);
        REQUIRE(same1 == same2_view);
        REQUIRE(same2 == same1_view);
        REQUIRE(same1 != diff1_view);
        REQUIRE(same1 != diff2_view);
        REQUIRE(same1 != empty1_view);

        REQUIRE_FALSE(same1 != same1_view);
        REQUIRE_FALSE(empty1 != empty2_view);
        REQUIRE_FALSE(same1 != same2_view);
        REQUIRE_FALSE(same2 != same1_view);
        REQUIRE_FALSE(same1 == diff1_view);
        REQUIRE_FALSE(same1 == diff2_view);
        REQUIRE_FALSE(same1 == empty1_view);
    }

    SECTION("View == Vec") {

        REQUIRE(same1_view == same1);
        REQUIRE(empty1_view == empty2);
        REQUIRE(same1_view == same2);
        REQUIRE(same2_view == same1);
        REQUIRE(same1_view != diff1);
        REQUIRE(same1_view != diff2);
        REQUIRE(same1_view != empty1);

        REQUIRE_FALSE(same1_view != same1);
        REQUIRE_FALSE(empty1_view != empty2);
        REQUIRE_FALSE(same1_view != same2);
        REQUIRE_FALSE(same2_view != same1);
        REQUIRE_FALSE(same1_view == diff1);
        REQUIRE_FALSE(same1_view == diff2);
        REQUIRE_FALSE(same1_view == empty1);
    }
}

TEST_CASE("PtrVecView stdlib integration", "[ptrvecview]") {
    auto v = OwnPtrVec<int>::make(1);
    auto view = v.view();

    SECTION("data") {
        REQUIRE(std::data(view) == view.data());
    }
    SECTION("begin") {
        REQUIRE(std::begin(view) == view.begin());
        REQUIRE(std::rbegin(view) == view.rbegin());
        REQUIRE(std::cbegin(view) == view.cbegin());
        REQUIRE(std::crbegin(view) == view.crbegin());
    }
    SECTION("end") {
        REQUIRE(std::end(view) == view.end());
        REQUIRE(std::rend(view) == view.rend());
        REQUIRE(std::cend(view) == view.cend());
        REQUIRE(std::crend(view) == view.crend());
    }
    SECTION("size") {
        REQUIRE(std::size(view) == view.size());
    }

    SECTION("swap") {
        auto v_std1 = OwnPtrVec<int>::make(10, 11);
        auto v_std2 = OwnPtrVec<int>::make(9);
        auto v_sw1 = OwnPtrVec<int>::make(10, 11);
        auto v_sw2 = OwnPtrVec<int>::make(9);

        auto std1 = v_std1.view();
        auto std2 = v_std2.view();
        auto sw1 = v_sw1.view();
        auto sw2 = v_sw2.view();

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
        auto ac = OwnPtrVec<int>::make(10, 10, 10, 10);
        PtrVecView<int> ac_view = ac.view();
        auto res = std::accumulate(std::begin(ac_view), std::end(ac_view), 0, [](int acc, int const *val) {
            return acc + *val;
        });
        REQUIRE(res == 40);
    }
}
