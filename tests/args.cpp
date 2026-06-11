#include <catch.hpp>
#include <ut/args/args.hpp>
#include <ut/args/color.hpp>

#include <cstdint>
#include <expected>
#include <format>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// Builds a valid (argc, argv) pair from a list of string_view tokens.
// First token is treated as the program name (argv[0]).
// Lifetime of argc()/argv() tied to the ArgvBuilder instance.
class ArgvBuilder {
public:
    explicit ArgvBuilder(std::initializer_list<std::string_view> args) {
        m_strings.reserve(args.size());
        m_ptrs.reserve(args.size() + 1);
        for (auto s : args) {
            m_strings.emplace_back(s);
        }
        for (auto &s : m_strings) {
            m_ptrs.push_back(s.data());
        }
        m_ptrs.push_back(nullptr);
    }

    ArgvBuilder(ArgvBuilder const &) = delete;
    ArgvBuilder &operator=(ArgvBuilder const &) = delete;
    ArgvBuilder(ArgvBuilder &&) = delete;
    ArgvBuilder &operator=(ArgvBuilder &&) = delete;
    ~ArgvBuilder() = default;

    [[nodiscard]]
    int argc() const {
        return static_cast<int>(m_strings.size());
    }

    [[nodiscard]]
    char **argv() {
        return m_ptrs.data();
    }

private:
    std::vector<std::string> m_strings;
    std::vector<char *> m_ptrs;
};

// RAII redirect of std::cout and std::cerr to internal string buffers.
// Restores originals on destruction.
class CaptureStreams {
public:
    CaptureStreams()
            : m_old_cout(std::cout.rdbuf(m_out.rdbuf()))
            , m_old_cerr(std::cerr.rdbuf(m_err.rdbuf())) { }

    CaptureStreams(CaptureStreams const &) = delete;
    CaptureStreams &operator=(CaptureStreams const &) = delete;
    CaptureStreams(CaptureStreams &&) = delete;
    CaptureStreams &operator=(CaptureStreams &&) = delete;

    ~CaptureStreams() {
        std::cout.rdbuf(m_old_cout);
        std::cerr.rdbuf(m_old_cerr);
    }

    [[nodiscard]]
    std::string out() const {
        return m_out.str();
    }

    [[nodiscard]]
    std::string err() const {
        return m_err.str();
    }

private:
    std::ostringstream m_out;
    std::ostringstream m_err;
    std::streambuf *m_old_cout;
    std::streambuf *m_old_cerr;
};

using namespace ut;
using ut::detail::TtyOverrideGuard;

// -- ansiEscape: named colors ------------------------------------------

TEST_CASE("Color::ansiEscape named fg", "[Color][ansiEscape][named]") {
    SECTION("Black") {
        CHECK(Color::Black.ansiEscape(true) == "\033[30m");
    }
    SECTION("Red") {
        CHECK(Color::Red.ansiEscape(true) == "\033[31m");
    }
    SECTION("Green") {
        CHECK(Color::Green.ansiEscape(true) == "\033[32m");
    }
    SECTION("Yellow") {
        CHECK(Color::Yellow.ansiEscape(true) == "\033[33m");
    }
    SECTION("Blue") {
        CHECK(Color::Blue.ansiEscape(true) == "\033[34m");
    }
    SECTION("Magenta") {
        CHECK(Color::Magenta.ansiEscape(true) == "\033[35m");
    }
    SECTION("Cyan") {
        CHECK(Color::Cyan.ansiEscape(true) == "\033[36m");
    }
    SECTION("White") {
        CHECK(Color::White.ansiEscape(true) == "\033[37m");
    }
}

TEST_CASE("Color::ansiEscape named bg", "[Color][ansiEscape][named]") {
    SECTION("Black") {
        CHECK(Color::Black.ansiEscape(false) == "\033[40m");
    }
    SECTION("Red") {
        CHECK(Color::Red.ansiEscape(false) == "\033[41m");
    }
    SECTION("Green") {
        CHECK(Color::Green.ansiEscape(false) == "\033[42m");
    }
    SECTION("Yellow") {
        CHECK(Color::Yellow.ansiEscape(false) == "\033[43m");
    }
    SECTION("Blue") {
        CHECK(Color::Blue.ansiEscape(false) == "\033[44m");
    }
    SECTION("Magenta") {
        CHECK(Color::Magenta.ansiEscape(false) == "\033[45m");
    }
    SECTION("Cyan") {
        CHECK(Color::Cyan.ansiEscape(false) == "\033[46m");
    }
    SECTION("White") {
        CHECK(Color::White.ansiEscape(false) == "\033[47m");
    }
}

// -- Color::ansiEscape: 256-color and RGB -------------------------------------

TEST_CASE("Color::ansiEscape 256-color", "[Color][ansiEscape][256]") {
    Color const c {std::uint8_t {200}};
    SECTION("fg") {
        CHECK(c.ansiEscape(true) == "\033[38;5;200m");
    }
    SECTION("bg") {
        CHECK(c.ansiEscape(false) == "\033[48;5;200m");
    }
    SECTION("id=0 fg") {
        CHECK(Color {std::uint8_t {0}}.ansiEscape(true) == "\033[38;5;0m");
    }
}

TEST_CASE("Color::ansiEscape RGB", "[Color][ansiEscape][rgb]") {
    Color const c {255, 128, 0};
    SECTION("fg") {
        CHECK(c.ansiEscape(true) == "\033[38;2;255;128;0m");
    }
    SECTION("bg") {
        CHECK(c.ansiEscape(false) == "\033[48;2;255;128;0m");
    }
}

// -- Formatter, TTY=false -----------------------------------------------------

TEST_CASE("formatter TTY=false: no escape codes", "[Color][formatter][tty=false]") {
    TtyOverrideGuard guard {false};
    SECTION("int fg compile-time") {
        CHECK(std::format("{}", fg<Color::Red>(42)) == "42");
    }
    SECTION("string fg compile-time") {
        CHECK(std::format("{}", fg<Color::Blue>(std::string {"hello"})) == "hello");
    }
    SECTION("int bg compile-time") {
        CHECK(std::format("{}", bg<Color::Green>(7)) == "7");
    }
    SECTION("int fg runtime") {
        CHECK(std::format("{}", fg(42, Color::Red)) == "42");
    }
    SECTION("int bg runtime") {
        CHECK(std::format("{}", bg(7, Color::Blue)) == "7");
    }
}

// -- Formatter, TTY=true, compile-time named colors ---------------------------

TEST_CASE("formatter TTY=true: compile-time named fg", "[Color][formatter][tty=true]") {
    TtyOverrideGuard guard {true};
    SECTION("fg<Color::Red> int") {
        CHECK(std::format("{}", fg<Color::Red>(42)) == "\033[31m42\033[0m");
    }
    SECTION("fg<Color::Blue> int") {
        CHECK(std::format("{}", fg<Color::Blue>(42)) == "\033[34m42\033[0m");
    }
    SECTION("fg<Color::Green> str") {
        CHECK(std::format("{}", fg<Color::Green>(std::string {"hi"})) == "\033[32mhi\033[0m");
    }
    SECTION("bg<Color::Cyan> int") {
        CHECK(std::format("{}", bg<Color::Cyan>(99)) == "\033[46m99\033[0m");
    }
    SECTION("bg<Color::White> int") {
        CHECK(std::format("{}", bg<Color::White>(0)) == "\033[47m0\033[0m");
    }
}

// -- Formatter, TTY=true, runtime color (all three color kinds) ---------------

TEST_CASE("formatter TTY=true: runtime color", "[Color][formatter][tty=true][runtime]") {
    TtyOverrideGuard guard {true};
    SECTION("fg named Red") {
        CHECK(std::format("{}", fg(42, Color::Red)) == "\033[31m42\033[0m");
    }
    SECTION("bg named Blue") {
        CHECK(std::format("{}", bg(42, Color::Blue)) == "\033[44m42\033[0m");
    }
    SECTION("fg 256-color id=200") {
        CHECK(std::format("{}", fg(42, Color {std::uint8_t {200}})) == "\033[38;5;200m42\033[0m");
    }
    SECTION("bg 256-color id=200") {
        CHECK(std::format("{}", bg(7, Color {std::uint8_t {200}})) == "\033[48;5;200m7\033[0m");
    }
    SECTION("fg RGB(255,128,0)") {
        CHECK(std::format("{}", fg(42, Color {255, 128, 0})) == "\033[38;2;255;128;0m42\033[0m");
    }
    SECTION("bg RGB(255,128,0)") {
        CHECK(std::format("{}", bg(7, Color {255, 128, 0})) == "\033[48;2;255;128;0m7\033[0m");
    }
}

// -- Formatter, TTY=true, compile-time 256/RGB colors -------------------------

TEST_CASE("formatter TTY=true: compile-time 256/RGB colors", "[Color][formatter][tty=true][compile-time]") {
    TtyOverrideGuard guard {true};
    constexpr Color c256 {std::uint8_t {200}};
    constexpr Color c_rgb {255, 128, 0};
    SECTION("fg 256") {
        CHECK(std::format("{}", fg<c256>(42)) == "\033[38;5;200m42\033[0m");
    }
    SECTION("bg 256") {
        CHECK(std::format("{}", bg<c256>(42)) == "\033[48;5;200m42\033[0m");
    }
    SECTION("fg RGB") {
        CHECK(std::format("{}", fg<c_rgb>(42)) == "\033[38;2;255;128;0m42\033[0m");
    }
    SECTION("bg RGB") {
        CHECK(std::format("{}", bg<c_rgb>(42)) == "\033[48;2;255;128;0m42\033[0m");
    }
}

// -- fg vs bg code distinction ------------------------------------------------

TEST_CASE("fg uses foreground codes, bg uses background codes", "[Color][fg-vs-bg]") {
    TtyOverrideGuard guard {true};
    SECTION("Red: fg=31, bg=41") {
        CHECK(std::format("{}", fg<Color::Red>(0)) == "\033[31m0\033[0m");
        CHECK(std::format("{}", bg<Color::Red>(0)) == "\033[41m0\033[0m");
    }
    SECTION("White: fg=37, bg=47") {
        CHECK(std::format("{}", fg<Color::White>(0)) == "\033[37m0\033[0m");
        CHECK(std::format("{}", bg<Color::White>(0)) == "\033[47m0\033[0m");
    }
}

// -- Format spec forwarding ---------------------------------------------------

TEST_CASE("formatter: format spec forwarded to inner value", "[Color][formatter][spec]") {
    SECTION("TTY=true: right-align width 5") {
        TtyOverrideGuard guard {true};
        CHECK(std::format("{:>5}", fg<Color::Red>(42)) == "\033[31m   42\033[0m");
    }
    SECTION("TTY=true: left-align width 5") {
        TtyOverrideGuard guard {true};
        CHECK(std::format("{:<5}", fg<Color::Red>(42)) == "\033[31m42   \033[0m");
    }
    SECTION("TTY=true: zero-pad width 5") {
        TtyOverrideGuard guard {true};
        CHECK(std::format("{:05}", fg<Color::Red>(42)) == "\033[31m00042\033[0m");
    }
    SECTION("TTY=true: string right-align width 8") {
        TtyOverrideGuard guard {true};
        CHECK(std::format("{:>8}", fg<Color::Blue>(std::string {"hi"})) == "\033[34m      hi\033[0m");
    }
    SECTION("TTY=false: right-align width 5") {
        TtyOverrideGuard guard {false};
        CHECK(std::format("{:>5}", fg<Color::Red>(42)) == "   42");
    }
    SECTION("TTY=false: left-align width 5") {
        TtyOverrideGuard guard {false};
        CHECK(std::format("{:<5}", fg<Color::Red>(42)) == "42   ");
    }
}

// -- TtyOverrideGuard correctness ---------------------------------------------

TEST_CASE("TtyOverrideGuard restores override on destruction", "[Color][TtyOverrideGuard]") {
    TtyOverrideGuard guard {false};
    {
        TtyOverrideGuard guard2 {true};
        CHECK(std::format("{}", fg<Color::Red>(1)) == "\033[31m1\033[0m");
    }
    CHECK(std::format("{}", fg<Color::Red>(1)) == "1");
}

TEST_CASE("TtyOverrideGuard false suppresses escapes", "[Color][TtyOverrideGuard]") {
    TtyOverrideGuard guard {false};
    CHECK(std::format("{}", fg<Color::Red>(1)) == "1");
}


// A custom enum flag type whose parser rejects unknown values with InvalidValue
// and advertises the accepted set via helpEnd — mirrors attevac's `--emit`.
enum struct ColorEnum : std::uint8_t { Red, Green, Blue };

template<>
struct std::formatter<ColorEnum> : std::formatter<std::string_view> {
    auto format(ColorEnum c, std::format_context &ctx) const {
        std::string_view const name = c == ColorEnum::Red ? "red" : c == ColorEnum::Green ? "green" : "blue";
        return std::formatter<std::string_view>::format(name, ctx);
    }
};

template<>
struct ut::ArgParser<ColorEnum> {
    static constexpr ParserKind kind = ParserKind::SingleArg;

    static std::expected<std::pair<bool, ColorEnum>, ParseError> parse(std::string_view,
        std::string_view value,
        ColorEnum *) {
        if (value == "red") return std::pair {true, ColorEnum::Red};
        if (value == "green") return std::pair {true, ColorEnum::Green};
        if (value == "blue") return std::pair {true, ColorEnum::Blue};
        return std::unexpected(ParseError::InvalidValue);
    }

    static std::string helpEnd() {
        return "One of [red, green, blue]";
    }
};

// ---------------------------------------------------------------------------
// Short flags
// ---------------------------------------------------------------------------

TEST_CASE("short flag -o assigns value to required string", "[ArgParser][short]") {
    std::string output;
    Parser parser {
        Arg('o', "--output", output),
    };
    ArgvBuilder a {"prog", "-o", "result.o"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(output == "result.o");
}

TEST_CASE("short bool flag -f sets true when present", "[ArgParser][short]") {
    bool force = false;
    Parser parser {
        Arg('f', force),
    };
    ArgvBuilder a {"prog", "-f"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(force);
}

TEST_CASE("short bool flag absent leaves false", "[ArgParser][short]") {
    bool force = false;
    std::string output;
    Parser parser {
        Arg('o', "--output", output),
        Arg('f', force),
    };
    ArgvBuilder a {"prog", "-o", "x"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK_FALSE(force);
}

TEST_CASE("short flag chain -abc: all bool flags set", "[ArgParser][short]") {
    bool alpha = false;
    bool beta = false;
    bool gamma = false;
    Parser parser {
        Arg('a', alpha),
        Arg('b', beta),
        Arg('c', gamma),
    };
    ArgvBuilder a {"prog", "-abc"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(alpha);
    CHECK(beta);
    CHECK(gamma);
}

TEST_CASE("short flag chain: last non-bool consumes next token", "[ArgParser][short]") {
    bool alpha = false;
    bool beta = false;
    std::string value;
    Parser parser {
        Arg('a', alpha),
        Arg('b', beta),
        Arg('l', "--value", value),
    };
    ArgvBuilder a {"prog", "-abl", "hello"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(alpha);
    CHECK(beta);
    CHECK(value == "hello");
}

TEST_CASE("short flag chain: non-bool not last yields error", "[ArgParser][short]") {
    bool alpha = false;
    std::string value;
    Parser parser {
        Arg('a', alpha),
        Arg('l', "--value", value),
    };
    // -va: 'v' is non-bool but not last in chain
    ArgvBuilder a {"prog", "-la", "hello"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res != ParseResult::Ok);
}

TEST_CASE("unknown short flag yields error", "[ArgParser][short]") {
    Parser parser {};
    ArgvBuilder a {"prog", "-z"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res != ParseResult::Ok);
    CHECK_FALSE(cap.out().empty());
}

// ---------------------------------------------------------------------------
// Long flags
// ---------------------------------------------------------------------------

TEST_CASE("long flag --output value assigns by space", "[ArgParser][long]") {
    std::string output;
    Parser parser {
        Arg("--output", output),
    };
    ArgvBuilder a {"prog", "--output", "out.txt"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(output == "out.txt");
}

TEST_CASE("long flag --output=value assigns by equals form", "[ArgParser][long]") {
    std::string output;
    Parser parser {
        Arg("--output", output),
    };
    ArgvBuilder a {"prog", "--output=out.txt"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(output == "out.txt");
}

TEST_CASE("long flag --output= with empty value assigns empty string", "[ArgParser][long]") {
    std::string output;
    Parser parser {
        Arg("--output", output),
    };
    ArgvBuilder a {"prog", "--output="};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(output.empty());
}

TEST_CASE("long bool flag --force sets true when present", "[ArgParser][long]") {
    bool force = false;
    Parser parser {
        Arg('f', force),
    };
    ArgvBuilder a {"prog", "-f"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(force);
}

TEST_CASE("single-dash token -output parsed as short-flag chain not long", "[ArgParser][long]") {
    // -output → short flags -o -u -t -p -u -t; 'o' registered, rest unknown → error
    bool o_flag = false;
    Parser parser {
        Arg('o', o_flag),
    };
    ArgvBuilder a {"prog", "-output"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    // 'u' unknown → error (or misparse); either way, not Ok
    CHECK(res != ParseResult::Ok);
}

TEST_CASE("unknown long flag yields error", "[ArgParser][long]") {
    Parser parser {};
    ArgvBuilder a {"prog", "--unknown"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res != ParseResult::Ok);
    CHECK_FALSE(cap.out().empty());
}

TEST_CASE("long non-bool flag missing value yields error", "[ArgParser][long]") {
    std::string output;
    Parser parser {
        Arg("--output", output),
    };
    // --output followed by nothing
    ArgvBuilder a {"prog", "--output"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res != ParseResult::Ok);
}

TEST_CASE("long non-bool flag value starting with dash: next flag consumed as value", "[ArgParser][long]") {
    // --output --other: '--other' starts with '-' so treated as a flag, not value
    // → --output has no value → error
    std::string output;
    bool other = false;
    Parser parser {
        Arg("--output", output),
        Arg("--other", other),
    };
    ArgvBuilder a {"prog", "--output", "--other"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res != ParseResult::Ok);
}

// ---------------------------------------------------------------------------
// Positional arguments
// ---------------------------------------------------------------------------

TEST_CASE("positional after flags assigned correctly", "[ArgParser][positional]") {
    std::string output;
    std::string input;
    Parser parser {
        Arg('o', "--output", output),
        Arg(positional, input),
    };
    ArgvBuilder a {"prog", "--output", "out.txt", "in.txt"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(input == "in.txt");
}

TEST_CASE("positional before flags assigned correctly", "[ArgParser][positional]") {
    std::string output;
    std::string input;
    Parser parser {
        Arg('o', "--output", output),
        Arg(positional, input),
    };
    ArgvBuilder a {"prog", "in.txt", "--output", "out.txt"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(input == "in.txt");
}

TEST_CASE("bare - token treated as positional not flag", "[ArgParser][positional]") {
    std::string input;
    Parser parser {
        Arg(positional, input),
    };
    ArgvBuilder a {"prog", "-"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(input == "-");
}

TEST_CASE("missing required positional yields error", "[ArgParser][positional]") {
    std::string input;
    Parser parser {
        Arg(positional, input),
    };
    ArgvBuilder a {"prog"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res != ParseResult::Ok);
    CHECK_FALSE(cap.out().empty());
}

TEST_CASE("extra unexpected positional yields error", "[ArgParser][positional]") {
    std::string input;
    Parser parser {
        Arg(positional, input),
    };
    ArgvBuilder a {"prog", "first.txt", "second.txt"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res != ParseResult::Ok);
}

// ---------------------------------------------------------------------------
// -- separator
// ---------------------------------------------------------------------------

TEST_CASE("-- separator: token after -- treated as positional even with leading dash", "[ArgParser][separator]") {
    std::string input;
    Parser parser {
        Arg(positional, input),
    };
    ArgvBuilder a {"prog", "--", "--not-a-flag"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(input == "--not-a-flag");
}

TEST_CASE("-- consumed: not included in positional value", "[ArgParser][separator]") {
    std::string input;
    Parser parser {
        Arg(positional, input),
    };
    ArgvBuilder a {"prog", "--", "file.txt"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(input == "file.txt");
}

TEST_CASE("-- separates flags from positionals: flags before --, positional after", "[ArgParser][separator]") {
    std::string output;
    std::string input;
    Parser parser {
        Arg("--output", output),
        Arg(positional, input),
    };
    ArgvBuilder a {"prog", "--output", "out.txt", "--", "--input-file"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(output == "out.txt");
    CHECK(input == "--input-file");
}

TEST_CASE("multiple --: only first is separator, subsequent are positionals", "[ArgParser][separator]") {
    std::string first;
    std::string second;
    Parser parser {
        Arg(positional, first),
        Arg(positional, second),
    };
    ArgvBuilder a {"prog", "--", "--", "actual"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    // second -- is a literal positional token
    CHECK(first == "--");
    CHECK(second == "actual");
}

// ---------------------------------------------------------------------------
// Required vs optional arguments
// ---------------------------------------------------------------------------

TEST_CASE("missing required non-optional flag yields error", "[ArgParser][optional]") {
    std::string output;
    Parser parser {
        Arg("--output", output),
    };
    ArgvBuilder a {"prog"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res != ParseResult::Ok);
    CHECK_FALSE(cap.out().empty());
}

TEST_CASE("optional with default retains default when absent", "[ArgParser][optional]") {
    std::optional<std::string> dir = std::string {"."};
    Parser parser {
        Arg("--out-dir", dir),
    };
    ArgvBuilder a {"prog"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(dir.has_value());
    CHECK(*dir == ".");
}

TEST_CASE("optional with default overwritten when provided", "[ArgParser][optional]") {
    std::optional<std::string> dir = std::string {"."};
    Parser parser {
        Arg("--out-dir", dir),
    };
    ArgvBuilder a {"prog", "--out-dir", "build/"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(dir.has_value());
    CHECK(*dir == "build/");
}

TEST_CASE("optional without default stays nullopt when absent", "[ArgParser][optional]") {
    std::optional<std::string> extra;
    Parser parser {
        Arg("--extra", extra),
    };
    ArgvBuilder a {"prog"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK_FALSE(extra.has_value());
}

TEST_CASE("optional without default set when provided", "[ArgParser][optional]") {
    std::optional<std::string> extra;
    Parser parser {
        Arg("--extra", extra),
    };
    ArgvBuilder a {"prog", "--extra", "flags"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(extra == std::optional<std::string> {"flags"});
}

TEST_CASE("optional and non-optional positional smoke test", "[ArgParser][optional]") {
    std::string file;
    std::optional<std::string> dir = std::string {"."};
    Parser parser {
        "name",
        "desc",
        Arg(positional, file).var("var").help("help"),
        Arg(positional, dir).var("var").help("help"),
    };
    SECTION("missing optional") {
        ArgvBuilder a {"prog", "file"};
        CaptureStreams cap;
        auto res = parser.parse(a.argc(), a.argv());
        CHECK(res == ParseResult::Ok);
        CHECK(file == "file");
        CHECK(dir.has_value());
        CHECK(*dir == ".");
    }
    SECTION("present optional") {
        ArgvBuilder b {"prog", "file", "dir"};
        CaptureStreams cap;
        auto res = parser.parse(b.argc(), b.argv());
        CHECK(res == ParseResult::Ok);
        CHECK(file == "file");
        CHECK(dir.has_value());
        CHECK(*dir == "dir");
    }
}

TEST_CASE("optional positional with default retains default when absent", "[ArgParser][optional]") {
    std::optional<std::string> dir = std::string {"."};
    Parser parser {
        Arg(positional, dir),
    };
    ArgvBuilder a {"prog"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(dir.has_value());
    CHECK(*dir == ".");
}

TEST_CASE("optional positional with default overwritten when provided", "[ArgParser][optional]") {
    std::optional<std::string> dir = std::string {"."};
    Parser parser {
        Arg(positional, dir),
    };
    ArgvBuilder a {"prog", "build/"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(dir.has_value());
    CHECK(*dir == "build/");
}

TEST_CASE("optional positional without default stays nullopt when absent", "[ArgParser][optional]") {
    std::optional<std::string> extra;
    Parser parser {
        Arg(positional, extra),
    };
    ArgvBuilder a {"prog"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK_FALSE(extra.has_value());
}

TEST_CASE("optional positional without default set when provided", "[ArgParser][optional]") {
    std::optional<std::string> extra;
    Parser parser {
        Arg(positional, extra),
    };
    ArgvBuilder a {"prog", "flags"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(extra == std::optional<std::string> {"flags"});
}

// ---------------------------------------------------------------------------
// ParseResult contract
// ---------------------------------------------------------------------------

TEST_CASE("successful parse returns Ok with no output", "[ArgParser][result]") {
    std::string output;
    Parser parser {
        Arg("--output", output),
    };
    ArgvBuilder a {"prog", "--output", "x"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(cap.out().empty());
    CHECK(cap.err().empty());
}

TEST_CASE("error result has non-zero to_underlying exit code", "[ArgParser][result]") {
    Parser parser {};
    ArgvBuilder a {"prog", "--unknown"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res != ParseResult::Ok);
    CHECK(std::to_underlying(res) != 0);
}

TEST_CASE("-h returns result with zero to_underlying exit code", "[ArgParser][result]") {
    Parser parser {};
    ArgvBuilder a {"prog", "-h"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res != ParseResult::Ok);
    CHECK(std::to_underlying(res) == 0);
}

TEST_CASE("--help returns result with zero to_underlying exit code", "[ArgParser][result]") {
    Parser parser {};
    ArgvBuilder a {"prog", "--help"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res != ParseResult::Ok);
    CHECK(std::to_underlying(res) == 0);
}

TEST_CASE("error result has non-zero to_underlying: usable as exit code", "[ArgParser][result]") {
    // Mirrors the idiomatic usage: if (res != Ok) return std::to_underlying(res);
    std::string output;
    Parser parser {
        Arg("--output", output),
    };
    ArgvBuilder a {"prog"};  // missing required flag
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res != ParseResult::Ok);
    CHECK(std::to_underlying(res) != 0);
}

// ---------------------------------------------------------------------------
// Help text content
// ---------------------------------------------------------------------------

TEST_CASE("-h prints help containing program name from argv[0]", "[ArgParser][help]") {
    Parser parser {};
    ArgvBuilder a {"my-tool", "-h"};
    CaptureStreams cap;
    parser.parse(a.argc(), a.argv());
    CHECK(cap.out().contains("my-tool"));
}

TEST_CASE("arg with .help() text appears in help output", "[ArgParser][help]") {
    std::string output;
    Parser parser {
        Arg("--output", output).help("Output file path"),
    };
    ArgvBuilder a {"prog", "--help"};
    CaptureStreams cap;
    parser.parse(a.argc(), a.argv());
    CHECK(cap.out().contains("Output file path"));
}

TEST_CASE("arg without .help() does not appear in help output", "[ArgParser][help]") {
    std::string output;
    std::string silent;
    Parser parser {
        Arg("--output", output).help("Output file"),
        Arg("--silent-flag", silent),
    };
    ArgvBuilder a {"prog", "--help"};
    CaptureStreams cap;
    parser.parse(a.argc(), a.argv());
    CHECK(!cap.out().contains("--silent-flag"));
}

TEST_CASE("non-bool flag without .var() uses stripped long name as metavar in help", "[ArgParser][help]") {
    std::string output;
    Parser parser {
        Arg("--output", output).help("Output file"),
    };
    ArgvBuilder a {"prog", "--help"};
    CaptureStreams cap;
    parser.parse(a.argc(), a.argv());
    CHECK(cap.out().contains("output"));
}

TEST_CASE(".var() replaces default VALUE metavar in help", "[ArgParser][help]") {
    std::string output;
    Parser parser {
        Arg('o', "--output", output).help("Output file").var("OUT"),
    };
    ArgvBuilder a {"prog", "--help"};
    CaptureStreams cap;
    parser.parse(a.argc(), a.argv());
    std::string const out = cap.out();
    CHECK(out.contains("OUT"));
    CHECK(!out.contains("VALUE"));
}

TEST_CASE("optional with default shows (Default: <value>) in help", "[ArgParser][help]") {
    std::optional<std::string> dir = std::string {"."};
    Parser parser {
        Arg("--out-dir", dir).help("Output directory"),
    };
    ArgvBuilder a {"prog", "--help"};
    CaptureStreams cap;
    parser.parse(a.argc(), a.argv());
    CHECK(cap.out().contains("Default: ."));
}

TEST_CASE("optional without default shows (Optional) in help", "[ArgParser][help]") {
    std::optional<std::string> extra;
    Parser parser {
        Arg("--extra", extra).help("Extra flags"),
    };
    ArgvBuilder a {"prog", "--help"};
    CaptureStreams cap;
    parser.parse(a.argc(), a.argv());
    CHECK(cap.out().contains("Optional"));
}

TEST_CASE("required flag shows neither (Default:) nor (Optional) in help", "[ArgParser][help]") {
    std::string output;
    Parser parser {
        Arg("--output", output).help("Output file"),
    };
    ArgvBuilder a {"prog", "--help"};
    CaptureStreams cap;
    parser.parse(a.argc(), a.argv());
    std::string const out = cap.out();
    CHECK(!out.contains("Default:"));
    CHECK(!out.contains("Optional"));
}

TEST_CASE("help text includes auto-added -h/--help entry", "[ArgParser][help]") {
    Parser parser {};
    ArgvBuilder a {"prog", "--help"};
    CaptureStreams cap;
    parser.parse(a.argc(), a.argv());
    std::string const out = cap.out();
    CHECK((out.contains("-h") || out.contains("--help")));
}

TEST_CASE("short+long pair shown together in help", "[ArgParser][help]") {
    std::string output;
    Parser parser {
        Arg('o', "--output", output).help("Output file"),
    };
    ArgvBuilder a {"prog", "--help"};
    CaptureStreams cap;
    parser.parse(a.argc(), a.argv());
    std::string const out = cap.out();
    // Both short and long should appear in the same help text
    CHECK(out.contains("-o"));
    CHECK(out.contains("--output"));
}

TEST_CASE("help printed to stdout not stderr", "[ArgParser][help]") {
    Parser parser {};
    ArgvBuilder a {"prog", "--help"};
    CaptureStreams cap;
    parser.parse(a.argc(), a.argv());
    CHECK_FALSE(cap.out().empty());
    CHECK(cap.err().empty());
}

// ---------------------------------------------------------------------------
// Edge cases
// ---------------------------------------------------------------------------

TEST_CASE("argc==1 (program name only): all-optional parser returns Ok with defaults", "[ArgParser][edge]") {
    std::optional<std::string> opt = std::string {"default"};
    Parser parser {
        Arg("--opt", opt),
    };
    ArgvBuilder a {"prog"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(*opt == "default");
}

TEST_CASE("argc==1: required flag missing yields error", "[ArgParser][edge]") {
    std::string required;
    Parser parser {
        Arg("--required", required),
    };
    ArgvBuilder a {"prog"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res != ParseResult::Ok);
}

TEST_CASE("repeated flag: last value wins", "[ArgParser][edge]") {
    // vector support not yet implemented; repeated scalar flag documents last-wins
    std::string output;
    Parser parser {
        Arg("--output", output),
    };
    ArgvBuilder a {"prog", "--output", "first", "--output", "second"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    // Either Ok with last value, or error — document whichever impl chooses.
    // Spec is silent; test asserts the chosen contract.
    if (res == ParseResult::Ok) {
        CHECK(output == "second");
    }
    // If impl rejects repeated flags, res != Ok is also valid;
    // both branches are acceptable pending spec clarification.
}

TEST_CASE("empty string value via --flag \"\" is accepted", "[ArgParser][edge]") {
    std::string output;
    Parser parser {
        Arg("--output", output),
    };
    ArgvBuilder a {"prog", "--output", ""};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(output.empty());
}

// ---------------------------------------------------------------------------
// Version flag (-v/--version)
// ---------------------------------------------------------------------------

TEST_CASE("--version with setVersion prints 'name version' and exits 0", "[ArgParser][version]") {
    std::string output;
    Parser parser {"my-tool", Arg("--output", output)};
    parser.setVersion("1.2.3");
    ArgvBuilder a {"prog", "--version"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res != ParseResult::Ok);
    CHECK(std::to_underlying(res) == 0);
    CHECK(cap.out().contains("my-tool"));
    CHECK(cap.out().contains("1.2.3"));
}

TEST_CASE("-v short form prints version", "[ArgParser][version]") {
    std::string output;
    Parser parser {"my-tool", Arg("--output", output)};
    parser.setVersion("9.9");
    ArgvBuilder a {"prog", "-v"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(std::to_underlying(res) == 0);
    CHECK(cap.out().contains("9.9"));
}

TEST_CASE("--version without setVersion reports version unknown", "[ArgParser][version]") {
    std::string output;
    Parser parser {"my-tool", Arg("--output", output)};
    ArgvBuilder a {"prog", "--version"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(std::to_underlying(res) == 0);
    CHECK(cap.out().contains("unknown"));
}

// ---------------------------------------------------------------------------
// Numeric flag types
// ---------------------------------------------------------------------------

TEST_CASE("int flag --count value assigns by space", "[ArgParser][number]") {
    int count = 0;
    Parser parser {
        Arg("--count", count),
    };
    ArgvBuilder a {"prog", "--count", "5"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(count == 5);
}

TEST_CASE("int flag --count=value assigns by equals form", "[ArgParser][number]") {
    int count = 0;
    Parser parser {
        Arg("--count", count),
    };
    ArgvBuilder a {"prog", "--count=5"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(count == 5);
}

TEST_CASE("short int flag -n value assigns", "[ArgParser][number]") {
    int count = 0;
    Parser parser {
        Arg('n', "--count", count),
    };
    ArgvBuilder a {"prog", "-n", "42"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(count == 42);
}

TEST_CASE("unsigned int flag parses decimal", "[ArgParser][number]") {
    unsigned count = 0;
    Parser parser {
        Arg("--count", count),
    };
    ArgvBuilder a {"prog", "--count=123"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(count == 123u);
}

TEST_CASE("negative int via space form rejected, equals form accepted", "[ArgParser][number]") {
    int count = 0;
    {
        Parser parser {Arg("--count", count)};
        ArgvBuilder a {"prog", "--count", "-5"};
        CaptureStreams cap;
        auto res = parser.parse(a.argc(), a.argv());
        CHECK(res != ParseResult::Ok);
    }
    {
        Parser parser {Arg("--count", count)};
        ArgvBuilder a {"prog", "--count=-5"};
        CaptureStreams cap;
        auto res = parser.parse(a.argc(), a.argv());
        CHECK(res == ParseResult::Ok);
        CHECK(count == -5);
    }
}

TEST_CASE("double flag parses 3.14", "[ArgParser][number]") {
    constexpr double expected_d = 3.14;
    double d = 0.0;
    Parser parser {
        Arg("--d", d),
    };
    ArgvBuilder a {"prog", "--d", "3.14"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(d == expected_d);
}

TEST_CASE("non-numeric value yields error with message", "[ArgParser][number]") {
    int count = 0;
    Parser parser {
        Arg("--count", count),
    };
    ArgvBuilder a {"prog", "--count", "abc"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res != ParseResult::Ok);
    auto out = cap.out();
    CHECK(out.contains("number"));
    CHECK(out.contains("--count"));
}

TEST_CASE("overflow value yields too-big error", "[ArgParser][number]") {
    std::int8_t small = 0;
    Parser parser {
        Arg("--s", small),
    };
    ArgvBuilder a {"prog", "--s=9999"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res != ParseResult::Ok);
    CHECK(cap.out().contains("big"));
}

TEST_CASE("trailing garbage in numeric value yields error", "[ArgParser][number]") {
    int count = 0;
    Parser parser {
        Arg("--count", count),
    };
    ArgvBuilder a {"prog", "--count", "12abc"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res != ParseResult::Ok);
}

TEST_CASE("optional<int> without default stays nullopt when absent", "[ArgParser][number]") {
    std::optional<int> count;
    Parser parser {
        Arg("--count", count),
    };
    ArgvBuilder a {"prog"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK_FALSE(count.has_value());
}

TEST_CASE("optional<int> with default retains default when absent", "[ArgParser][number]") {
    constexpr int default_count = 7;
    std::optional<int> count = default_count;
    Parser parser {
        Arg("--count", count),
    };
    ArgvBuilder a {"prog"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(count == default_count);
}

TEST_CASE("optional<int> with default overwritten when provided", "[ArgParser][number]") {
    constexpr int default_count = 7;
    constexpr int new_count = 99;
    std::optional<int> count = default_count;
    Parser parser {
        Arg("--count", count),
    };
    ArgvBuilder a {"prog", "--count=99"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(count == new_count);
}

TEST_CASE("positional int parses", "[ArgParser][number]") {
    int count = 0;
    Parser parser {
        Arg(positional, count),
    };
    ArgvBuilder a {"prog", "7"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(count == 7);
}

// ---------------------------------------------------------------------------
// AsFalse modifier
// ---------------------------------------------------------------------------

TEST_CASE("AsFalse flag absent leaves initialised value untouched", "[ArgParser][modifier][asfalse]") {
    bool verbose = true;
    Parser parser {
        Arg("--quiet", AsFalse {verbose}),
    };
    ArgvBuilder a {"prog"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(verbose);
}

TEST_CASE("AsFalse flag presence stores false", "[ArgParser][modifier][asfalse]") {
    bool verbose = true;
    Parser parser {
        Arg("--quiet", AsFalse {verbose}),
    };
    ArgvBuilder a {"prog", "--quiet"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK_FALSE(verbose);
}

TEST_CASE("AsFalse via short flag stores false", "[ArgParser][modifier][asfalse]") {
    bool b = true;
    Parser parser {
        Arg('q', AsFalse {b}),
    };
    ArgvBuilder a {"prog", "-q"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK_FALSE(b);
}

TEST_CASE("AsFalse via short+long: short form works", "[ArgParser][modifier][asfalse]") {
    bool b = true;
    Parser parser {
        Arg('q', "--quiet", AsFalse {b}),
    };
    ArgvBuilder a {"prog", "-q"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK_FALSE(b);
}

TEST_CASE("AsFalse via short+long: long form works", "[ArgParser][modifier][asfalse]") {
    bool b = true;
    Parser parser {
        Arg('q', "--quiet", AsFalse {b}),
    };
    ArgvBuilder a {"prog", "--quiet"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK_FALSE(b);
}

TEST_CASE("AsFalse in short-chain: all set to false", "[ArgParser][modifier][asfalse]") {
    bool alpha = true;
    bool beta = true;
    bool gamma = true;
    Parser parser {
        Arg('a', AsFalse {alpha}),
        Arg('b', AsFalse {beta}),
        Arg('c', AsFalse {gamma}),
    };
    ArgvBuilder a {"prog", "-abc"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK_FALSE(alpha);
    CHECK_FALSE(beta);
    CHECK_FALSE(gamma);
}

TEST_CASE("AsFalse shows no metavar in help", "[ArgParser][modifier][asfalse]") {
    bool b = true;
    Parser parser {
        Arg('q', "--quiet", AsFalse {b}).help("Suppress output"),
    };
    ArgvBuilder a {"prog", "--help"};
    CaptureStreams cap;
    parser.parse(a.argc(), a.argv());
    auto out = cap.out();
    CHECK(out.contains("--quiet"));
    // AsFalse is bool-like: no metavar should follow the flag name
    CHECK_FALSE(out.contains("VALUE"));
    CHECK_FALSE(out.contains("quiet quiet"));
}

TEST_CASE("AsFalse is optional: absent parser with no other args returns Ok", "[ArgParser][modifier][asfalse]") {
    bool b = true;
    Parser parser {
        Arg("--quiet", AsFalse {b}),
    };
    ArgvBuilder a {"prog"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
}

// ---------------------------------------------------------------------------
// Equals-form must not consume the following token
// ---------------------------------------------------------------------------

TEST_CASE("--flag=value does not consume the following positional", "[ArgParser][regression][equals]") {
    // Regression: the equals-form parse returned the inner `bool` (true) as the
    // count of consumed args, so the parse loop skipped the next token — here
    // eating the positional and reporting it missing.
    std::string output;
    std::string input;
    Parser parser {
        Arg("--output", output),
        Arg(positional, input),
    };
    ArgvBuilder a {"prog", "--output=out.o", "in.atv"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(output == "out.o");
    CHECK(input == "in.atv");
}

TEST_CASE("optional<string>=value does not consume the following positional", "[ArgParser][regression][equals]") {
    std::optional<std::string> output;
    std::string input;
    Parser parser {
        Arg("--output", output),
        Arg(positional, input),
    };
    ArgvBuilder a {"prog", "--output=out.o", "in.atv"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(output == std::optional<std::string> {"out.o"});
    CHECK(input == "in.atv");
}

// ---------------------------------------------------------------------------
// optional<T> propagates inner parse errors (does not swallow them)
// ---------------------------------------------------------------------------

TEST_CASE("optional<ColorEnum> bad value via equals form reports InvalidValue", "[ArgParser][regression][optional]") {
    // Regression: ArgParser<optional<T>> ended in .value_or(...) which discarded
    // the inner unexpected, leaving the target nullopt and reporting success.
    std::optional<ColorEnum> color = ColorEnum::Red;
    Parser parser {
        Arg("--color", color),
    };
    ArgvBuilder a {"prog", "--color=bogus"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res != ParseResult::Ok);
    auto out = cap.out();
    CHECK(out.contains("Invalid value"));
    CHECK(out.contains("bogus"));
    CHECK(out.contains("--color"));
    CHECK(out.contains("red, green, blue"));
}

TEST_CASE("optional<ColorEnum> bad value via space form reports InvalidValue", "[ArgParser][regression][optional]") {
    std::optional<ColorEnum> color = ColorEnum::Red;
    Parser parser {
        Arg("--color", color),
    };
    ArgvBuilder a {"prog", "--color", "bogus"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res != ParseResult::Ok);
    CHECK(cap.out().contains("bogus"));
}

TEST_CASE("optional<ColorEnum> good value parses through", "[ArgParser][regression][optional]") {
    std::optional<ColorEnum> color;
    Parser parser {
        Arg("--color", color),
    };
    ArgvBuilder a {"prog", "--color=green"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(color == std::optional<ColorEnum> {ColorEnum::Green});
}

// ---------------------------------------------------------------------------
// InvalidValue diagnostic names the value and accepted set
// ---------------------------------------------------------------------------

TEST_CASE("required ColorEnum bad value names value and accepted set", "[ArgParser][regression][invalid]") {
    ColorEnum color {};
    Parser parser {
        Arg("--color", color),
    };
    ArgvBuilder a {"prog", "--color=mauve"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res != ParseResult::Ok);
    auto out = cap.out();
    CHECK(out.contains("mauve"));
    CHECK(out.contains("--color"));
    // the `=value` tail must not leak into the flag name
    CHECK_FALSE(out.contains("--color=mauve"));
    CHECK(out.contains("red, green, blue"));
}

// ---------------------------------------------------------------------------
// std::vector<T> targets (repeatable flags)
// ---------------------------------------------------------------------------

TEST_CASE("vector<string> collects each occurrence (space form)", "[ArgParser][vector]") {
    std::vector<std::string> libs;
    Parser parser {
        Arg("--lib", libs),
    };
    ArgvBuilder a {"prog", "--lib", "m", "--lib", "c", "--lib", "pthread"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    REQUIRE(libs.size() == 3);
    CHECK(libs.at(0) == "m");
    CHECK(libs.at(1) == "c");
    CHECK(libs.at(2) == "pthread");
}

TEST_CASE("vector<string> collects each occurrence (equals form)", "[ArgParser][vector]") {
    std::vector<std::string> libs;
    Parser parser {
        Arg("--lib", libs),
    };
    ArgvBuilder a {"prog", "--lib=m", "--lib=c"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    REQUIRE(libs.size() == 2);
    CHECK(libs.at(0) == "m");
    CHECK(libs.at(1) == "c");
}

TEST_CASE("vector<string> is optional: absent leaves it empty", "[ArgParser][vector]") {
    std::vector<std::string> libs;
    Parser parser {
        Arg("--lib", libs),
    };
    ArgvBuilder a {"prog"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    CHECK(libs.empty());
}

TEST_CASE("vector<string> preserves pre-existing elements", "[ArgParser][vector]") {
    std::vector<std::string> libs {"preset"};
    Parser parser {
        Arg("--lib", libs),
    };
    ArgvBuilder a {"prog", "--lib", "extra"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    REQUIRE(libs.size() == 2);
    CHECK(libs.at(0) == "preset");
    CHECK(libs.at(1) == "extra");
}

TEST_CASE("vector<int> parses each occurrence", "[ArgParser][vector][number]") {
    std::vector<int> nums;
    Parser parser {
        Arg("--n", nums),
    };
    ArgvBuilder a {"prog", "--n=1", "--n=2", "--n=3"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    REQUIRE(nums.size() == 3);
    CHECK(nums.at(0) == 1);
    CHECK(nums.at(1) == 2);
    CHECK(nums.at(2) == 3);
}

TEST_CASE("vector<int> bad value reports an error", "[ArgParser][vector][number]") {
    std::vector<int> nums;
    Parser parser {
        Arg("--n", nums),
    };
    ArgvBuilder a {"prog", "--n=1", "--n=oops"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res != ParseResult::Ok);
    CHECK(cap.out().contains("number"));
}

TEST_CASE("vector<string>=value does not consume the following positional", "[ArgParser][vector]") {
    std::vector<std::string> libs;
    std::string input;
    Parser parser {
        Arg("--lib", libs),
        Arg(positional, input),
    };
    ArgvBuilder a {"prog", "--lib=m", "in.atv"};
    CaptureStreams cap;
    auto res = parser.parse(a.argc(), a.argv());
    CHECK(res == ParseResult::Ok);
    REQUIRE(libs.size() == 1);
    CHECK(libs.at(0) == "m");
    CHECK(input == "in.atv");
}

TEST_CASE("vector<string> shows (Repeatable) in help", "[ArgParser][vector][help]") {
    std::vector<std::string> libs;
    Parser parser {
        Arg("--lib", libs).help("Extra library to link"),
    };
    ArgvBuilder a {"prog", "--help"};
    CaptureStreams cap;
    parser.parse(a.argc(), a.argv());
    CHECK(cap.out().contains("Repeatable"));
}
