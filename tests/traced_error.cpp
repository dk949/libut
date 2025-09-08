#include <catch.hpp>
#include <catch2/catch_test_macros.hpp>
#include <ut/traced_error/traced_error.hpp>
using namespace std::string_view_literals;

void throwsTracedError() {
    throw ut::TracedError("Hello {}", "world");
};

TEST_CASE("Basic usage", "[traced_error]") {

    try {
        throwsTracedError();
    } catch (ut::TracedError const &t) {
        auto trace = t.trace();
        INFO(trace);
        REQUIRE(!trace.empty());
        REQUIRE(t.what() == "Hello world"sv);
    }
}

class MyError : public ut::TracedError {
public:
    template<typename... Args>
    MyError(std::format_string<Args...> fmt, Args &&...args)
            : TracedError(1, fmt, std::forward<Args>(args)...) { }
};

void throwsMyError() {
    throw MyError("Hello {}", "world");
};

TEST_CASE("Inheriting from exception", "[traced_error]") {

    try {
        throwsMyError();
    } catch (MyError const &t) {
        auto trace = t.trace();
        INFO(trace);
        REQUIRE(!trace.empty());
        REQUIRE(t.what() == "Hello world"sv);
    } catch (ut::TracedError const &t) {
        FAIL("This handler should never be entered");
    }
    try {
        throwsMyError();
    } catch (ut::TracedError const &t) {
        auto trace = t.trace();
        INFO(trace);
        REQUIRE(!trace.empty());
        REQUIRE(t.what() == "Hello world"sv);
    }
}
