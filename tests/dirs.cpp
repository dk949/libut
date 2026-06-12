#include <catch.hpp>
#include <ut/dirs/dirs.hpp>

#include <cstdlib>
#include <filesystem>
#include <optional>
#include <string>
#include <thread>
#include <utility>

#if !defined(_WIN32)
#    include <unistd.h>
#endif

namespace {

void setEnvVar(char const *name, char const *value) {
#if defined(_WIN32)
    _putenv_s(name, value);
#else
    setenv(name, value, 1);
#endif
}

void unsetEnvVar(char const *name) {
#if defined(_WIN32)
    // an empty value removes the variable on Windows
    _putenv_s(name, "");
#else
    unsetenv(name);
#endif
}

std::optional<std::string> getEnvVar(char const *name) {
#if defined(_MSC_VER)
#    pragma warning(push)
#    pragma warning(disable : 4996)  // std::getenv is fine here, it's read once and copied immediately
#endif
    if (auto const *value = std::getenv(name)) return value;
#if defined(_MSC_VER)
#    pragma warning(pop)
#endif
    return std::nullopt;
}

// Temporarily sets (or clears) an environment variable, restoring its
// previous value (or absence) on destruction.
class EnvVar {
public:
    EnvVar(std::string name, char const *value) : m_name(std::move(name)) {
        m_old = getEnvVar(m_name.c_str());
        setEnvVar(m_name.c_str(), value);
    }

    EnvVar(std::string name, std::nullopt_t) : m_name(std::move(name)) {
        m_old = getEnvVar(m_name.c_str());
        unsetEnvVar(m_name.c_str());
    }

    EnvVar(EnvVar const &) = delete;
    EnvVar &operator=(EnvVar const &) = delete;

    ~EnvVar() {
        if (m_old)
            setEnvVar(m_name.c_str(), m_old->c_str());
        else
            unsetEnvVar(m_name.c_str());
    }

private:
    std::string m_name;
    std::optional<std::string> m_old;
};

// ut::dir caches its results in a thread_local, so each lookup that should
// observe a fresh environment needs to run on its own thread.
template<typename F>
std::filesystem::path inFreshThread(F &&getter) {
    std::filesystem::path result;
    std::thread([&] { result = getter(); }).join();
    return result;
}

}  // namespace

#if !defined(_WIN32)

TEST_CASE("dir functions use XDG environment variables when set", "[dirs]") {
    EnvVar xdg_config("XDG_CONFIG_HOME", "/tmp/ut_dirs_test/config");
    EnvVar xdg_cache("XDG_CACHE_HOME", "/tmp/ut_dirs_test/cache");
    EnvVar xdg_data("XDG_DATA_HOME", "/tmp/ut_dirs_test/data");
    EnvVar xdg_state("XDG_STATE_HOME", "/tmp/ut_dirs_test/state");
    EnvVar xdg_runtime("XDG_RUNTIME_DIR", "/tmp/ut_dirs_test/runtime");

    CHECK(inFreshThread([] { return ut::dir::config(); }) == "/tmp/ut_dirs_test/config");
    CHECK(inFreshThread([] { return ut::dir::cache(); }) == "/tmp/ut_dirs_test/cache");
    CHECK(inFreshThread([] { return ut::dir::data(); }) == "/tmp/ut_dirs_test/data");
    CHECK(inFreshThread([] { return ut::dir::state(); }) == "/tmp/ut_dirs_test/state");
    CHECK(inFreshThread([] { return ut::dir::runtime(); }) == "/tmp/ut_dirs_test/runtime");
    // logs() shares XDG_STATE_HOME with state()
    CHECK(inFreshThread([] { return ut::dir::logs(); }) == "/tmp/ut_dirs_test/state");
}

TEST_CASE("dir functions fall back to XDG defaults under $HOME", "[dirs]") {
    EnvVar xdg_config("XDG_CONFIG_HOME", std::nullopt);
    EnvVar xdg_cache("XDG_CACHE_HOME", std::nullopt);
    EnvVar xdg_data("XDG_DATA_HOME", std::nullopt);
    EnvVar xdg_state("XDG_STATE_HOME", std::nullopt);
    EnvVar xdg_runtime("XDG_RUNTIME_DIR", std::nullopt);
    EnvVar home("HOME", "/tmp/ut_dirs_test_home");

    auto const home_path = std::filesystem::path("/tmp/ut_dirs_test_home");

    CHECK(inFreshThread([] { return ut::dir::config(); }) == home_path / ".config");
    CHECK(inFreshThread([] { return ut::dir::cache(); }) == home_path / ".cache");
    CHECK(inFreshThread([] { return ut::dir::data(); }) == home_path / ".local" / "share");
    CHECK(inFreshThread([] { return ut::dir::state(); }) == home_path / ".local" / "state");
    CHECK(inFreshThread([] { return ut::dir::logs(); }) == home_path / ".local" / "state");

    // XDG_RUNTIME_DIR's fallback is an absolute path, independent of $HOME.
    CHECK(
        inFreshThread([] { return ut::dir::runtime(); })
        == std::filesystem::path("/run/user") / std::to_string(getuid())
    );
}

TEST_CASE("config falls back to / when neither XDG_CONFIG_HOME nor HOME are set", "[dirs]") {
    EnvVar xdg_config("XDG_CONFIG_HOME", std::nullopt);
    EnvVar home("HOME", std::nullopt);

    CHECK(inFreshThread([] { return ut::dir::config(); }) == std::filesystem::path("/") / ".config");
}

#else  // _WIN32

TEST_CASE("dir functions use AppData environment variables when set", "[dirs]") {
    EnvVar appdata("APPDATA", "C:\\ut_dirs_test\\Roaming");
    EnvVar localappdata("LOCALAPPDATA", "C:\\ut_dirs_test\\Local");
    EnvVar temp("TEMP", "C:\\ut_dirs_test\\Temp");

    CHECK(inFreshThread([] { return ut::dir::config(); }) == "C:\\ut_dirs_test\\Roaming");
    CHECK(inFreshThread([] { return ut::dir::cache(); }) == "C:\\ut_dirs_test\\Local");
    CHECK(inFreshThread([] { return ut::dir::data(); }) == "C:\\ut_dirs_test\\Roaming");
    CHECK(inFreshThread([] { return ut::dir::state(); }) == "C:\\ut_dirs_test\\Local");
    CHECK(inFreshThread([] { return ut::dir::runtime(); }) == "C:\\ut_dirs_test\\Temp");
    CHECK(inFreshThread([] { return ut::dir::logs(); }) == "C:\\ut_dirs_test\\Local");
}

TEST_CASE("dir functions fall back to AppData defaults under %USERPROFILE%", "[dirs]") {
    EnvVar appdata("APPDATA", std::nullopt);
    EnvVar localappdata("LOCALAPPDATA", std::nullopt);
    EnvVar temp("TEMP", std::nullopt);
    EnvVar userprofile("USERPROFILE", "C:\\ut_dirs_test_home");

    auto const home_path = std::filesystem::path("C:\\ut_dirs_test_home");

    CHECK(inFreshThread([] { return ut::dir::config(); }) == home_path / "AppData" / "Roaming");
    CHECK(inFreshThread([] { return ut::dir::cache(); }) == home_path / "AppData" / "Local");
    CHECK(inFreshThread([] { return ut::dir::data(); }) == home_path / "AppData" / "Roaming");
    CHECK(inFreshThread([] { return ut::dir::state(); }) == home_path / "AppData" / "Local");
    CHECK(inFreshThread([] { return ut::dir::runtime(); }) == home_path / "AppData" / "Local" / "Temp");
    CHECK(inFreshThread([] { return ut::dir::logs(); }) == home_path / "AppData" / "Local");
}

TEST_CASE("config falls back to C:/ when neither APPDATA nor USERPROFILE are set", "[dirs]") {
    EnvVar appdata("APPDATA", std::nullopt);
    EnvVar userprofile("USERPROFILE", std::nullopt);

    CHECK(inFreshThread([] { return ut::dir::config(); }) == std::filesystem::path("C:/") / "AppData" / "Roaming");
}

#endif  // _WIN32

TEST_CASE("dir lookups are cached per-thread", "[dirs]") {
#if defined(_WIN32)
    char const *const env_name = "APPDATA";
#else
    char const *const env_name = "XDG_CONFIG_HOME";
#endif

    EnvVar config_var(env_name, "/tmp/ut_dirs_test/first");

    std::filesystem::path first;
    std::filesystem::path second;
    std::thread([&] {
        first = ut::dir::config();
        setEnvVar(env_name, "/tmp/ut_dirs_test/second");
        second = ut::dir::config();
    }).join();

    CHECK(first == "/tmp/ut_dirs_test/first");
    CHECK(second == first);
}
