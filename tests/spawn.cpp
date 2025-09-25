#ifdef __unix__
#    include <catch.hpp>
#    include <catch2/catch_test_macros.hpp>
#    include <ut/spawn/spawn.hpp>

TEST_CASE("commandExists", "[spawn]") {
    REQUIRE(ut::commandExists("ls"));
    REQUIRE(ut::commandExists("/usr/bin/ls"));
    REQUIRE(!ut::commandExists("./ls"));
    REQUIRE(!ut::commandExists("aaaaaaa"));
}

TEST_CASE("simple spawn", "[spawn]") {
    auto proc = ut::spawn("cat", "/dev/null");
    REQUIRE(!!proc);
    auto res = proc.wait();
    REQUIRE(res);
    REQUIRE(!res->stdout().has_value());
    REQUIRE(!res->stderr().has_value());
    REQUIRE(res->code() == 0);
    REQUIRE(res->kind() == ut::StopKind::Exit);
}

TEST_CASE("spawn with redirect", "[spawn]") {
    auto proc = ut::spawn("cat", "-", ut::SpawnConfig {.stdin = "hello", .stdout = ut::OutPipe::String});
    REQUIRE(!!proc);
    auto res = proc.wait();
    REQUIRE(res);
    REQUIRE(res->stdout() == "hello");
    REQUIRE(!res->stderr().has_value());
    REQUIRE(res->code() == 0);
    REQUIRE(res->kind() == ut::StopKind::Exit);
}

TEST_CASE("shell", "[spawn]") {
    auto sh = ut::shell("echo 'stderr' 1>&2 && false", ut::SpawnConfig {.stderr = ut::OutPipe::String});
    REQUIRE(!!sh);
    auto res = sh.wait();
    REQUIRE(res);
    REQUIRE(res->stderr() == "stderr\n");
    REQUIRE(!res->stdout().has_value());
    REQUIRE(res->code() == 1);
    REQUIRE(res->kind() == ut::StopKind::Exit);
}

#endif  // __unix__
