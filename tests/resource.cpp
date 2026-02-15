#include <catch.hpp>
#include <ut/resource/resource.hpp>

struct Res { };

bool func_released = false;

void release(Res const &) {
    func_released = true;
}

TEST_CASE("resource value initialisation", "[resource]") {
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

TEST_CASE("resource delayed value initialisatoin", "[resource]") {

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

TEST_CASE("resource default destructor ctor", "[resource]") {
    static bool released_val;
    released_val = false;

    static bool released_ptr;
    released_ptr = false;

    {
        ut::Resource<int, decltype([](int) { released_val = true; })> val {1};
    }
    REQUIRE(released_val);

    {
        int i;
        ut::Resource<int *, decltype([](int *) { released_ptr = true; })> ptr {&i};
    }
    REQUIRE(released_ptr);
}

TEST_CASE("resource hasValue", "[resource]") {
    auto const release_value = [](int) {
    };

    auto const release_ptr = [](int const *) {
    };

    ut::Resource<int, decltype(release_value)> val_empty {release_value};
    ut::Resource<int *, decltype(release_ptr)> const ptr_empty {release_ptr};

    int v = 0;
    ut::Resource<int, decltype(release_value)> const val {v, release_value};
    ut::Resource<int *, decltype(release_ptr)> ptr {&v, release_ptr};

    SECTION("value") {
        REQUIRE(!val_empty.hasValue());
        REQUIRE(val.hasValue());
        REQUIRE(!val_empty);
        REQUIRE(val);
    }

    SECTION("pointer") {
        REQUIRE(!ptr_empty);
        REQUIRE(ptr);
        REQUIRE(!ptr_empty.hasValue());
        REQUIRE(ptr.hasValue());
    }
}

TEST_CASE("resource get", "[resource]") {
    auto const release_value = [](int) {
    };
    auto const release_ptr = [](int const *) {
    };

    int val = 23;
    ut::Resource res_val {val, release_value};
    ut::Resource const const_res_val {val, release_value};
    ut::Resource empty_res_val {release_value};

    int *ptr_val = reinterpret_cast<int *>(0x123456789ABCDEF0);
    int const *const_ptr_val = reinterpret_cast<int const *>(0xFEDCBA9876543210);

    ut::Resource res_ptr {ptr_val, release_ptr};
    ut::Resource const const_res_ptr {ptr_val, release_ptr};
    ut::Resource res_const_ptr {const_ptr_val, release_ptr};
    ut::Resource const const_res_const_ptr {const_ptr_val, release_ptr};

    ut::Resource empty_res_ptr {nullptr, release_ptr};


    SECTION("value") {
        REQUIRE(res_val.get() == val);
        REQUIRE(const_res_val.get() == val);
    }

    SECTION("pointer") {
        REQUIRE(res_ptr.get() == ptr_val);
        REQUIRE(const_res_ptr.get() == ptr_val);
        REQUIRE(res_const_ptr.get() == const_ptr_val);
        REQUIRE(const_res_const_ptr.get() == const_ptr_val);
    }
}

TEST_CASE("resource operator->", "[resource]") {
    struct S {
        int i;
    };

    auto const release_value = [](S) {
    };
    auto const release_ptr = [](S const *) {
    };

    S val {23};
    ut::Resource res_val {val, release_value};
    ut::Resource const const_res_val {val, release_value};

    SECTION("value") {
        REQUIRE(res_val->i == 23);
        REQUIRE(const_res_val->i == 23);
    }

    S *ptr_val = new S {42};
    S const *const_ptr_val = ptr_val;

    ut::Resource res_ptr {ptr_val, release_ptr};
    ut::Resource const const_res_ptr {ptr_val, release_ptr};
    ut::Resource res_const_ptr {const_ptr_val, release_ptr};
    ut::Resource const const_res_const_ptr {const_ptr_val, release_ptr};

    SECTION("pointer") {
        REQUIRE(res_ptr->i == 42);
        REQUIRE(const_res_ptr->i == 42);
        REQUIRE(res_const_ptr->i == 42);
        REQUIRE(const_res_const_ptr->i == 42);
    }
    delete ptr_val;
}

TEST_CASE("resource move ctor", "[resource]") {
    int val_released = 0;
    int ptr_released = 0;
    auto const release_value = [&](int) {
        REQUIRE(!val_released);
        val_released++;
    };

    auto const release_ptr = [&](int const *) {
        REQUIRE(!ptr_released);
        ptr_released = true;
    };

    int v = 0;
    SECTION("value") {
        {
            ut::Resource<int, decltype(release_value)> val1 {v, release_value};
            auto val2 = std::move(val1);
        }
        REQUIRE(val_released == 1);
    }
    SECTION("pointer") {
        {
            ut::Resource<int *, decltype(release_ptr)> ptr1 {&v, release_ptr};
            auto ptr2 = std::move(ptr1);
        }
        REQUIRE(ptr_released == 1);
    }
}

TEST_CASE("resource move operator=", "[resource]") {
    struct ReleaseVal {
        ReleaseVal(int *r)
                : released(r) { }

        ReleaseVal() = default;

        ReleaseVal(ReleaseVal const &) = delete;
        ReleaseVal(ReleaseVal &&) = default;
        ReleaseVal &operator=(ReleaseVal const &) = delete;
        ReleaseVal &operator=(ReleaseVal &&) = default;
        ~ReleaseVal() = default;
        int *released = nullptr;

        void operator()(int) {
            REQUIRE(!*released);
            (*released)++;
        }
    };

    struct ReleasePtr {
        ReleasePtr(int *r)
                : released(r) { }

        ReleasePtr() = default;

        int *released = nullptr;

        void operator()(int *) {
            REQUIRE(!*released);
            (*released)++;
        }
    };

    int v = 0;
    SECTION("value") {
        int released = 0;
        {
            ReleaseVal r {&released};
            ut::Resource<int, ReleaseVal> val1 {v, std::move(r)};
            ut::Resource<int, ReleaseVal> val2;
            val2 = std::move(val1);
        }
        REQUIRE(released == 1);
    }
    SECTION("pointer") {
        int released = 0;
        ReleasePtr r {&released};
        {
            ut::Resource<int *, ReleasePtr> val1 {&v, std::ref(r)};
            ut::Resource<int *, ReleasePtr> val2;
            val2 = std::move(val1);
        }
        REQUIRE(released == 1);
    }
}

TEST_CASE("resource malloced", "[resource]") {
    {
        [[maybe_unused]]
        auto m = ut::malloced(malloc(10));
    }
    SUCCEED("No memeory leak");


    {
        auto m = ut::malloced(malloc(12));
        free(m.takeOwnership());
    }
    SUCCEED("No double free");
}

TEST_CASE("default ctor", "[resource]") {
    static int released = 0;
    {
        ut::Resource<int, decltype([](int) { released++; })> res {};
        res.acquire(1);
    }
    REQUIRE(released == 1);
}

TEST_CASE("default constructable dtor + single arg ctor", "[resource]") {
    static int released = 0;
    {
        ut::Resource<int, decltype([](int) { released++; })> res {1};
    }
    REQUIRE(released == 1);
}
