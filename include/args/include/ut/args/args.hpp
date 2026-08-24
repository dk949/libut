#ifndef UT_ARGS_HPP
#define UT_ARGS_HPP


#include <ut/args/color.hpp>

#include <cassert>
#include <charconv>
#include <concepts>
#include <cstdint>
#include <expected>
#include <format>
#include <iostream>
#include <iterator>
#include <optional>
#include <print>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace ut {

// Detects whether T is a specialization of the single-argument template Spec
// (e.g. IsSpec<std::optional<int>, std::optional>).
template<typename T, template<typename...> typename Spec>
struct is_spec : std::false_type { };

template<typename T, template<typename...> typename Spec>
struct is_spec<Spec<T>, Spec> : std::true_type { };

template<typename T, template<typename...> typename Spec>
inline constexpr bool is_spec_v = is_spec<T, Spec>::value;

template<typename T, template<typename...> typename Spec>
concept IsSpec = is_spec_v<T, Spec>;

namespace detail {
    // Trips a compile-time error when reached in a consteval context (calling a
    // non-constexpr function from consteval is ill-formed). Used as a `throw`
    // replacement for compile-time validation under `-fno-exceptions`.
    // Intentionally left undefined: it is only ever reached in consteval
    // contexts, so it is never ODR-used at runtime and needs no definition.
    [[noreturn]]
    void consteval_error(char const *msg);
}  // namespace detail

enum struct ParseError : std::uint8_t {
    NotEnoughArgs,
    UnsupportedFormat,
    NotFound,
    TooManyPos,
    NotANumber,
    NumberTooBig,
    InvalidValue,
};

enum ParserKind : std::uint8_t { Generic, SingleArg, NoEql };

template<typename T, typename Self>
struct Modifier {
    using type = T;
protected:
    T *m_data;
public:
    // NOLINTNEXTLINE(bugprone-crtp-constructor-accessibility) -- this is used by derived class
    explicit(false) Modifier(T &data)  // cppcheck-suppress noExplicitConstructor
            : m_data(&data) { }

    T *asTarget() {
        return m_data;
    }

    Modifier &operator*() {
        return *this;
    }

    Modifier const &operator*() const {
        return *this;
    }

    // NOLINTNEXTLINE(misc-unconventional-assign-operator) -- this hooks into derived operator=
    Self &operator=(T const &inner) {
        return static_cast<Self &>(*this).modify(inner);
    }

    // NOLINTNEXTLINE(misc-unconventional-assign-operator) -- this hooks into derived operator=
    Self &operator=(T &&inner) {
        return static_cast<Self &>(*this).modify(std::move(inner));
    }

    friend Self;
};

template<typename T>
struct ArgParser {
    // static constexpr ParserKind kind;

    // std::expected<std::pair<int, T>, ParseError> parse(std::string_view arg, std::span<char *> next_args, T
    // *current); std::expected<std::pair<bool, T>, ParseError> parse(std::string_view arg, std::string_view value,
    // T *current); std::string helpEnd() const;
};

template<typename T>
concept HasParserMulti = ArgParser<T>::kind != ParserKind::SingleArg
                      && requires(ArgParser<T> parser, std::string_view arg, std::span<char *> next_args, T *current) {
                             {
                                 parser.parse(arg, next_args, current)
                             } -> std::same_as<std::expected<std::pair<int, T>, ParseError>>;
                         };

template<typename T>
concept HasParserEql = ArgParser<T>::kind != ParserKind::NoEql
                    && requires(ArgParser<T> parser, std::string_view arg, std::string_view value, T *current) {
                           {
                               parser.parse(arg, value, current)
                           } -> std::same_as<std::expected<std::pair<bool, T>, ParseError>>;
                       };

template<typename T>
concept HasParserHelp = requires(ArgParser<T> const parser) {
    { parser.helpEnd() } -> std::same_as<std::string>;
};

template<typename T>
concept HasArgParser = HasParserEql<T> || HasParserMulti<T>;

template<typename T>
concept IsModifier = std::derived_from<T, Modifier<typename T::type, T>>;

template<typename T, typename Other>
concept SameOrModifierOf = (IsModifier<T> && std::same_as<typename T::type, Other>) || std::same_as<T, Other>;

template<typename T>
concept ArgTarget = HasArgParser<T> || IsModifier<T>;

template<>
struct ArgParser<std::string> {
    static constexpr ParserKind kind = ParserKind::SingleArg;

    static std::expected<std::pair<bool, std::string>, ParseError> parse(std::string_view,
        std::string_view value,
        std::string *) {
        return std::pair {true, std::string {value}};
    }
};

template<typename T>
requires(std::integral<T> || std::floating_point<T>)
struct ArgParser<T> {
    static constexpr ParserKind kind = ParserKind::SingleArg;

    static std::expected<std::pair<bool, T>, ParseError> parse(std::string_view, std::string_view value, T *) {
        T out {};
        auto [ptr, ec] = std::from_chars(value.begin(), value.end(), out);
        if (ptr != value.end() || ec == std::errc::invalid_argument) return std::unexpected(ParseError::NotANumber);
        if (ec != std::errc {}) return std::unexpected(ParseError::NumberTooBig);
        return std::pair {true, out};
    }
};

template<>
struct ArgParser<bool> {
    static constexpr ParserKind kind = ParserKind::NoEql;

    static std::expected<std::pair<int, bool>, ParseError> parse(std::string_view, std::span<char *>, bool *) {
        return std::pair {0, true};
    }
};

template<typename T>
struct ArgParser<std::optional<T>> {
    static constexpr ParserKind kind = ArgParser<T>::kind;
    ArgParser<T> inner_parser;

    std::expected<std::pair<int, std::optional<T>>, ParseError> parse(std::string_view arg,
        std::span<char *> next_args,
        std::optional<T> *current)
    requires(HasParserMulti<T>)
    {
        T *inner_current = (current && current->has_value()) ? std::addressof(current->value()) : nullptr;
        // Propagate the inner parser's error verbatim: optionality means the flag
        // may be *absent* (Arg::m_required is cleared), not that a bad value is
        // silently dropped. A swallowed error here leaves the target nullopt and
        // reports success, so the driver later aborts dereferencing it.
        return std::move(inner_parser.parse(arg, next_args, inner_current))
            // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward) -- std::forward_like used instead
            .transform([]<typename Tt>(Tt &&value) {
            return std::pair {value.first, std::optional<T> {std::forward_like<Tt>(value.second)}};
        });
    }

    std::expected<std::pair<bool, std::optional<T>>, ParseError> parse(std::string_view arg,
        std::string_view value,
        std::optional<T> *current)
    requires(HasParserEql<T>)
    {
        T *inner_current = (current && current->has_value()) ? std::addressof(current->value()) : nullptr;
        // See the multi-arg overload above: propagate, do not swallow.
        return std::move(inner_parser.parse(arg, value, inner_current))
            // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward) -- std::forward_like used instead
            .transform([]<typename Tt>(Tt &&result) {
            return std::pair {result.first, std::optional<T> {std::forward_like<Tt>(result.second)}};
        });
    }

    [[nodiscard]]
    std::string helpEnd() const
    requires(HasParserHelp<T>)
    {
        return inner_parser.helpEnd();
    }
};

// A std::vector<T> target collects every occurrence of the flag: each parse
// appends one freshly-parsed T to the (moved-out) running vector.  Like
// optional/bool targets, a vector flag is always optional (an absent flag
// yields an empty vector).  Inner errors propagate (no swallowing).
template<typename T>
requires(!SameOrModifierOf<T, bool> && !IsSpec<T, std::optional>)
struct ArgParser<std::vector<T>> {
    static constexpr ParserKind kind = ArgParser<T>::kind;
    ArgParser<T> inner_parser;

    std::expected<std::pair<int, std::vector<T>>, ParseError> parse(std::string_view arg,
        std::span<char *> next_args,
        std::vector<T> *current)
    requires(HasParserMulti<T>)
    {
        return std::move(inner_parser.parse(arg, next_args, nullptr))
            // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward) -- std::forward_like used instead
            .transform([&]<typename Tt>(Tt &&value) {
            std::vector<T> out = current ? std::move(*current) : std::vector<T> {};
            out.push_back(std::forward_like<Tt>(value.second));
            return std::pair {value.first, std::move(out)};
        });
    }

    std::expected<std::pair<bool, std::vector<T>>, ParseError> parse(std::string_view arg,
        std::string_view value,
        std::vector<T> *current)
    requires(HasParserEql<T>)
    {
        return std::move(inner_parser.parse(arg, value, nullptr))
            // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward) -- std::forward_like used instead
            .transform([&]<typename Tt>(Tt &&result) {
            std::vector<T> out = current ? std::move(*current) : std::vector<T> {};
            out.push_back(std::forward_like<Tt>(result.second));
            return std::pair {result.first, std::move(out)};
        });
    }

    [[nodiscard]]
    std::string helpEnd() const
    requires(HasParserHelp<T>)
    {
        return inner_parser.helpEnd();
    }
};

template<typename T>
struct arg_parser_type;

template<HasArgParser T>
struct arg_parser_type<T> {
    using type = T;
};

template<IsModifier T>
struct arg_parser_type<T> {
    using type = typename T::type;
};

template<typename T>
using arg_parser_type_t = arg_parser_type<T>::type;

static constexpr struct Positional {
} positional;

struct FmtIndent {
    std::size_t init {};
    std::size_t help {};
    std::size_t line_len {};
    static constexpr auto min_help_len = 35;
};

struct ShortArg {
    char ch;

    consteval explicit(false) ShortArg(char c)  // cppcheck-suppress noExplicitConstructor
            : ch(c) {
        if (c == 'v' || c == 'h') detail::consteval_error("Use of reserved flag");
    }
};

struct LongArg {
    std::string_view sv;

    consteval explicit(false) LongArg(char const *s)  // cppcheck-suppress noExplicitConstructor
            : LongArg(std::string_view {s}) { }

    consteval explicit(false) LongArg(std::string_view s)  // cppcheck-suppress noExplicitConstructor
            : sv(s) {
        if (s == "--version" || s == "--help" || s == "--") detail::consteval_error("Use of reserved flag");
        if (!s.starts_with("--")) detail::consteval_error("Long arguments have to start with --");
    }
};

template<typename T>
struct AsFalse;

template<typename T>
AsFalse(T) -> AsFalse<T>;

template<>
struct AsFalse<bool> : Modifier<bool, AsFalse<bool>> {
    using Modifier::Modifier;

    AsFalse &modify(bool new_val) {
        *m_data = !new_val;
        return *this;
    }
};

template<>
struct AsFalse<std::optional<bool>> : Modifier<std::optional<bool>, AsFalse<std::optional<bool>>> {
    using Modifier::Modifier;

    AsFalse &modify(std::optional<bool> new_val) {
        if (new_val) *m_data = !*new_val;
        return *this;
    }
};

template<ArgTarget T>
struct Arg {
private:
    using VarT = std::conditional_t<SameOrModifierOf<T, bool>, std::monostate, std::string_view>;
    using StoredT = std::conditional_t<IsModifier<T>, T, T *>;
    using ArgParserT = ArgParser<arg_parser_type_t<T>>;
    char m_short;
    std::string_view m_long;
    std::string_view m_help;
    StoredT m_target;
    ArgParserT m_parser;
    [[no_unique_address]]
    VarT m_var;
    bool m_required = !IsSpec<arg_parser_type_t<T>, std::optional> && !IsSpec<arg_parser_type_t<T>, std::vector>
                   && !SameOrModifierOf<T, bool>;
    static constexpr auto parser_kind = ArgParserT::kind;

    template<IsSpec<Arg>... Ts>
    friend struct Parser;

    [[nodiscard]]
    std::string_view getVar() const {
        if constexpr (!SameOrModifierOf<T, bool>)
            return !m_var.empty()           ? m_var
                 : m_long.starts_with("--") ? m_long.substr(2)
                 : m_long.starts_with('-')  ? m_long.substr(1)
                 : !m_long.empty()          ? m_long
                                            : "VALUE";
        else
            return "";
    }

    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters) -- all three params serve distinct output roles
    void appendUsage(std::string &positionals, std::string &short_args, std::string &short_chain) const {
        if (m_short == 0 && m_long.empty()) {
            std::format_to(std::back_inserter(positionals),
                " {}{}{}",
                IsSpec<T, std::optional> ? "[" : "",
                fg<Color::Green>(getVar()),
                IsSpec<T, std::optional> ? "]" : "");
        } else if (m_short != 0) {
            if constexpr (!SameOrModifierOf<T, bool>) {
                short_args.push_back(' ');
                if constexpr (IsSpec<T, std::optional>) short_args.push_back('[');
                std::format_to(std::back_inserter(short_args),
                    "{}{} {}",
                    fg<Color::Green>("-"),
                    fg<Color::Green>(m_short),
                    fg<Color::Yellow>(getVar()));
                if constexpr (IsSpec<T, std::optional>) short_args.push_back(']');
            } else
                short_chain += m_short;
        }
    }

    [[nodiscard]]
    std::string formatHelp(FmtIndent indent) const {
        if (m_help.empty()) return "";
        auto const total_indent = indent.init + indent.help;
        auto const total_help = indent.line_len - total_indent;
        if (total_help < FmtIndent::min_help_len) {
            std::println(
                "Internal Warning: Help indentation too long to print message ({{.init: {}, .help: {}, .line_len: {}}})",
                indent.init,
                indent.help,
                indent.line_len);
        }
        auto fmt = std::format("{:{}}", " ", indent.init);
        std::size_t length = 0;
        if (m_short) {
            std::format_to(std::back_inserter(fmt), "{}{}", fg<Color::Green>("-"), fg<Color::Green>(m_short));
            length += 2;
        }
        if (!m_long.empty()) {
            std::format_to(std::back_inserter(fmt), "{}{}", m_short ? "," : "", fg<Color::Cyan>(m_long));
            length += (m_short ? 1 : 0) + m_long.size();
        }
        if constexpr (!SameOrModifierOf<T, bool>) {
            auto const var = getVar();
            std::format_to(std::back_inserter(fmt),
                "{}{}",
                (m_short || !m_long.empty()) ? " " : "",
                fg<Color::Yellow>(var));
            length += ((m_short || !m_long.empty()) ? 1 : 0) + var.size();
        }

        if (length + 1 >= indent.help)
            std::format_to(std::back_inserter(fmt), "\n{:{}}", " ", total_indent);
        else
            std::format_to(std::back_inserter(fmt), "{:{}}", " ", indent.help - length);

        std::format_to(std::back_inserter(fmt), "{}", m_help);
        if constexpr (HasParserHelp<T>)
            std::format_to(std::back_inserter(fmt), "\n{:{}}{}", " ", total_indent + (indent.init / 2), m_parser.helpEnd());

        if constexpr (IsSpec<T, std::optional>) {
            if (m_target->has_value())
                std::format_to(std::back_inserter(fmt),
                    "\n{:{}}(Default: {})",
                    " ",
                    total_indent + (indent.init / 2),
                    m_target->value());
            else
                std::format_to(std::back_inserter(fmt), "\n{:{}}(Optional)", " ", total_indent + (indent.init / 2));
        }
        if constexpr (IsSpec<T, std::vector>)
            std::format_to(std::back_inserter(fmt), "\n{:{}}(Repeatable)", " ", total_indent + (indent.init / 2));
        fmt += '\n';
        return fmt;
        // TODO(dk949): Handle line_len (split m_help on spaces with chunk length of at most total_help)
        //              Print each chunk with total_indent of indentation.
    }

    struct PrivateTag { };

    Arg(char short_, std::string_view long_, PrivateTag)
            : m_short(short_)
            , m_long(long_) { }

    Arg(char short_, std::string_view long_, T *target, PrivateTag)
    requires(HasArgParser<T>)
            : m_short(short_)
            , m_long(long_)
            , m_target(target) {
        if constexpr (std::same_as<T, bool>) *m_target = false;
    }

    Arg(char short_, std::string_view long_, T target, PrivateTag)
    requires(IsModifier<T>)
            : m_short(short_)
            , m_long(long_)
            , m_target(target) { }
public:

    Arg(ShortArg short_, LongArg long_, T &target)  // cppcheck-suppress passedByValue
    requires(HasArgParser<T>)
            : Arg(short_.ch, long_.sv, &target, PrivateTag {}) { }

    Arg(LongArg long_, T &target)  // cppcheck-suppress passedByValue
    requires(HasArgParser<T>)
            : Arg(0, long_.sv, &target, PrivateTag {}) { }

    Arg(ShortArg short_, T &target)
    requires(HasArgParser<T>)
            : Arg(short_.ch, "", &target, PrivateTag {}) { }

    Arg(Positional, T &target)
    requires(!SameOrModifierOf<T, bool> && HasArgParser<T>)
            : Arg(0, "", &target, PrivateTag {}) { }

    Arg(ShortArg short_, LongArg long_, T target)  // cppcheck-suppress passedByValue
    requires(IsModifier<T>)
            : Arg(short_.ch, long_.sv, target, PrivateTag {}) { }

    Arg(LongArg long_, T target)  // cppcheck-suppress passedByValue
    requires(IsModifier<T>)
            : Arg(0, long_.sv, target, PrivateTag {}) { }

    Arg(ShortArg short_, T target)
    requires(IsModifier<T>)
            : Arg(short_.ch, "", target, PrivateTag {}) { }

    Arg(Positional, T target)
    requires(IsModifier<T>)
            : Arg(0, "", target, PrivateTag {}) { }

    std::expected<int, ParseError> parse(std::string_view arg, std::span<char *> next_args) {
        if constexpr (parser_kind == ParserKind::SingleArg) {
            if (next_args.empty()) return std::unexpected(ParseError::NotEnoughArgs);
            std::string_view const val {next_args.front()};
            if ((m_short != 0 || !m_long.empty()) && val.starts_with('-') && val != "-")
                return std::unexpected(ParseError::NotEnoughArgs);
            return parse(arg, val).transform([](bool) { return 1; });
        } else {
            auto res = [&] {
                if constexpr (HasArgParser<T>) {
                    return m_parser.parse(arg, next_args, m_target);
                } else {
                    return m_parser.parse(arg, next_args, m_target.asTarget());
                }
            }();
            if (res) {
                *m_target = std::move(res->second);
                return res->first;
            } else {
                return std::unexpected(res.error());
            }
        }
    }

    std::expected<bool, ParseError> parse(std::string_view arg, std::string_view value) {
        if constexpr (parser_kind == ParserKind::NoEql) {
            return std::unexpected(ParseError::UnsupportedFormat);
        } else {
            if (auto res = m_parser.parse(arg, value, m_target)) {
                *m_target = std::move(res->second);
                return res->first;
            } else {
                return std::unexpected(res.error());
            }
        }
    }

    Arg &&help(std::string_view help_str) && {
        m_help = help_str;
        return std::move(*this);
    }

    Arg &&var(std::string_view var_str) &&
    requires(!SameOrModifierOf<T, bool>)
    {
        m_var = var_str;
        return std::move(*this);
    }
};

template<HasArgParser T>
Arg(char, std::string_view, T &) -> Arg<T>;
template<IsModifier T>
Arg(char, std::string_view, T) -> Arg<T>;

enum struct ParseResult : int8_t { Ok = -1 };

template<IsSpec<Arg>... Ts>
struct Parser {
    static constexpr auto default_indent = FmtIndent {
        .init = 4,
        .help = 30,
        .line_len = 80,
    };
private:
    std::string_view m_name;
    std::string_view m_desc;
    std::string_view m_version;
    std::tuple<std::pair<Ts, bool>...> m_args;

    // Context captured when an InvalidValue error surfaces, so printError can
    // name the offending value and (if the arg's parser exposes helpEnd) the
    // accepted set. Owning copies: helpEnd returns a temporary std::string.
    std::string m_bad_value;
    std::string m_bad_accepted;

    // TODO(dk949): Make this a part of the configuration
    FmtIndent m_indent = default_indent;

    template<typename... Args>
    [[nodiscard]]
    ParseResult error(int8_t err, std::format_string<Args...> fmt, Args &&...args) {
        std::println(std::cout, fmt, std::forward<Args>(args)...);
        return ParseResult {err};
    }

    ParseResult help() {
        auto help_str = std::format("{}", fg<Color::Magenta>(m_name));
        if (!m_desc.empty()) std::format_to(std::back_inserter(help_str), " - {}", fg<Color::Cyan>(m_desc));
        help_str.append("\n\n");
        {
            std::string positionals;
            std::string short_args;
            std::string short_chain;
            [&]<std::size_t... i>(std::index_sequence<i...>) {
                ([&] {
                    std::get<i>(m_args).first.appendUsage(positionals, short_args, short_chain);
                }(), ...);
            }(std::make_index_sequence<sizeof...(Ts)>());
            std::format_to(std::back_inserter(help_str),
                "{} {}{}",
                fg<Color::Blue>("Usage:"),
                fg<Color::Magenta>(m_name),
                positionals);
            if (!short_chain.empty())
                std::format_to(std::back_inserter(help_str),
                    " [{}{}]",
                    fg<Color::Green>("-"),
                    fg<Color::Green>(short_chain));
            help_str.append(short_args);
            help_str.append("\n\n");
        }
        [&]<std::size_t... i>(std::index_sequence<i...>) {
            ([&] {
                auto const &arg = std::get<i>(m_args).first;
                if (arg.m_short == 0 && arg.m_long.empty()) help_str.append(arg.formatHelp(m_indent));
            }(), ...);
        }(std::make_index_sequence<sizeof...(Ts)>());
        help_str.push_back('\n');
        [&]<std::size_t... i>(std::index_sequence<i...>) {
            ([&] {
                auto const &arg = std::get<i>(m_args).first;
                if (arg.m_short != 0 || !arg.m_long.empty()) help_str.append(arg.formatHelp(m_indent));
            }(), ...);
        }(std::make_index_sequence<sizeof...(Ts)>());
        help_str.append("\n");
        {
            help_str.append(Arg<bool>('h', std::string_view {"--help"}, Arg<bool>::PrivateTag {})
                    .help("Print this message and exit")
                    .formatHelp(m_indent));
            help_str.append(Arg<bool>('v', std::string_view {"--version"}, Arg<bool>::PrivateTag {})
                    .help("Print version and exit")
                    .formatHelp(m_indent));
        }
        return error(0, "{}\n", help_str);
    }

    ParseResult version() {
        if (m_version.empty()) return error(0, "{} (version unknown)", m_name);
        return error(0, "{} {}", m_name, m_version);
    }

    ParseResult handleMissing(std::string_view arg) {
        if (arg == "h" || arg == "-h" || arg == "--help") return help();
        if (arg == "v" || arg == "-v" || arg == "--version") return version();
        auto const *dash = arg.size() == 1 && arg != "-" ? "-" : "";
        return error(1, "Unknwon argument '{}{}'", dash, arg);
    }

    ParseResult printError(std::string_view arg, ParseError err) {
        // Show only the flag name, not the `=value` tail of an equals-form token.
        if (auto eq = arg.find('='); eq != std::string_view::npos) arg = arg.substr(0, eq);
        auto const *dash = arg.size() == 1 && arg != "-" ? "-" : "";
        switch (err) {
            using enum ParseError;
            case NotEnoughArgs: return error(1, "'{}{}' expected an argument", dash, arg);
            case UnsupportedFormat: return error(1, "'{}{}' does not support --arg=value format", dash, arg);
            case TooManyPos: return error(1, "Too many positional arguments");
            case NotANumber: return error(1, "Expected a number as '{}{}' argument", dash, arg);
            case NumberTooBig: return error(1, "Argument too big for '{}{}'", dash, arg);
            case InvalidValue:
                if (!m_bad_value.empty() && !m_bad_accepted.empty())
                    return error(1, "Invalid value '{}' for '{}{}'. {}", m_bad_value, dash, arg, m_bad_accepted);
                if (!m_bad_value.empty()) return error(1, "Invalid value '{}' for '{}{}'", m_bad_value, dash, arg);
                return error(1, "Invalid value for '{}{}'", dash, arg);
            case NotFound: return handleMissing(arg);
            default: std::unreachable();
        }
    }

    // Record the offending value (and accepted set, when the arg's parser
    // exposes helpEnd) for an InvalidValue diagnostic.
    template<typename A>
    void captureInvalid(A &arg_ref, std::string_view val) {  // cppcheck-suppress unusedPrivateFunction
        m_bad_value = std::string {val};
        if constexpr (requires { arg_ref.m_parser.helpEnd(); })
            m_bad_accepted = arg_ref.m_parser.helpEnd();
        else
            m_bad_accepted.clear();
    }

    // NOLINTNEXTLINE(readability-function-cognitive-complexity) -- TODO(dk949): split this
    std::expected<int, ParseError> runParser(std::string_view arg_, std::span<char *> rest) {
        enum { Double, Eql, Single, Pos } kind {};

        std::string_view value;  // cppcheck-suppress variableScope
        std::string_view arg = arg_;
        if (arg.empty()) {
            kind = Pos;
        } else if (arg.size() == 1) {
            kind = Single;
        } else if (arg.contains('=')) {
            kind = Eql;
            auto eql = arg.find('=');
            assert(eql != arg.npos);
            value = arg.substr(eql + 1);
            arg = arg.substr(0, eql);
        }
        std::expected<int, ParseError> out = std::unexpected(ParseError::NotFound);
        // NOLINTNEXTLINE(readability-function-cognitive-complexity) -- variadic match-dispatch fold; inherent
        [&]<std::size_t... i>(std::index_sequence<i...>) {
            (void)(([&] {
                auto &arg_ref = std::get<i>(m_args).first;
                auto &found = std::get<i>(m_args).second;
                // The value that would surface in an InvalidValue diagnostic: the
                // equals-form tail, else the next token (for the space-separated form).
                auto const bad = kind == Eql  ? value
                               : rest.empty() ? std::string_view {}
                                              : std::string_view {rest.front()};
                switch (kind) {
                    case Double:
                    case Eql:
                        if (arg_ref.m_long != arg) return true;
                        // Equals-form value lives in this token, so consume 0 extra args
                        // (returning the parser's `bool` here would skip the next arg).
                        out = kind == Eql ? arg_ref.parse(arg, value).transform([](bool) { return 0; })
                                          : arg_ref.parse(arg, rest);
                        break;
                    case Single:
                        if (arg_ref.m_short != arg.front()) return true;
                        out = arg_ref.parse(arg, rest);
                        break;
                    case Pos:
                        if (!(arg_ref.m_short == 0 && arg_ref.m_long.empty())) return true;
                        if (found) {
                            if (i == sizeof...(Ts) - 1 && !IsSpec<Ts, std::optional>)
                                out = std::unexpected(ParseError::TooManyPos);
                            return i != sizeof...(Ts) - 1;
                        }
                        out = arg_ref.parse(arg, rest);
                        break;
                    default: std::unreachable();
                }
                if (!out && out.error() == ParseError::InvalidValue) captureInvalid(arg_ref, bad);
                found = true;
                return false;
            }() && ...));
        }(std::make_index_sequence<sizeof...(Ts)>());
        return out;
    }
public:
    template<IsSpec<Arg>... Tts>
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters) -- name and desc are semantically distinct
    explicit Parser(std::string_view name, std::string_view desc, Tts &&...args)
            : m_name(name)
            , m_desc(desc)
            , m_args(std::pair {std::forward<Tts>(args), false}...) { }

    template<IsSpec<Arg>... Tts>
    explicit Parser(std::string_view name, Tts &&...args)
            : Parser(name, "", std::forward<Tts>(args)...) { }

    // Version string reported by `-v`/`--version`.  Set after construction since
    // the constructors are template-deduced over the argument specs.
    Parser &setVersion(std::string_view ver) {
        m_version = ver;
        return *this;
    }

    template<IsSpec<Arg>... Tts>
    explicit Parser(Tts &&...args)
            : Parser("", "", std::forward<Tts>(args)...) { }

    // NOLINTNEXTLINE(readability-function-cognitive-complexity) -- argument parsing dispatch; inherent complexity
    ParseResult parse(int argc, char **argv) {
        if (argc < 1) return error(1, "Expected argv[0] to be executable name");
        if (m_name.empty()) m_name = argv[0];
        std::span args {&argv[1], static_cast<std::size_t>(argc - 1)};
        bool skip = false;
        for (int i = 1; i < argc; ++i) {
            auto arg = std::string_view {argv[i]};
            if (!skip && arg == "--") {
                skip = true;
                continue;
            }
            std::expected<int, ParseError> res;
            char last = 0;  // Needs to be declared here to have the same lifetime as arg
            if (skip || arg == "-" || !arg.starts_with('-')) {
                res = runParser("", {&argv[i], 1});
                if (res && res.value() > 0) res = std::expected<int, ParseError> {res.value() - 1};
            } else if (arg.starts_with("--")) {
                res = runParser(arg, args.subspan(static_cast<std::size_t>(i)));
            } else {
                assert(arg.starts_with('-'));
                assert(arg.size() >= 2);
                auto inner = arg.substr(1, arg.size() - 2);
                last = arg.back();
                for (auto ch : inner) {
                    auto ch_arg = std::string_view {&ch, 1};
                    auto inner_res = runParser(ch_arg, {});
                    if (!inner_res) return printError(ch_arg, inner_res.error());
                    assert(inner_res.value() == 0);
                }
                arg = std::string_view {&last, 1};
                res = runParser(arg, args.subspan(static_cast<std::size_t>(i)));
            }
            if (!res) return printError(arg, res.error());
            i += res.value();
        }
        ParseResult required_result = ParseResult::Ok;
        [&]<std::size_t... idx>(std::index_sequence<idx...>) {
            (void)(([&]() -> bool {
                if (required_result != ParseResult::Ok) return false;
                auto const &[arg_obj, found] = std::get<idx>(m_args);
                if (!found && arg_obj.m_required) {
                    if (!arg_obj.m_long.empty()) {
                        required_result = error(1, "Missing required argument '{}'", arg_obj.m_long);
                    } else if (arg_obj.m_short != 0) {
                        required_result = error(1, "Missing required argument '-{}'", arg_obj.m_short);
                    } else {
                        required_result = error(1, "Missing required positional argument");
                    }
                    return false;
                }
                return true;
            }() && ...));
        }(std::make_index_sequence<sizeof...(Ts)>());
        return required_result;
    }
};

template<IsSpec<Arg>... Ts>
Parser(Ts &&...) -> Parser<Ts...>;

template<IsSpec<Arg>... Ts>
Parser(std::string_view, Ts &&...) -> Parser<Ts...>;

template<IsSpec<Arg>... Ts>
Parser(std::string_view, std::string_view, Ts &&...) -> Parser<Ts...>;

}  // namespace ut


#endif  // UT_ARGS_HPP
