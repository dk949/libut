#ifndef UT_SWITCHBOARD_HPP
#define UT_SWITCHBOARD_HPP

// TODO(dk949): Clean up API:
//                * Fix inconsistent naming
//                * Hide things in `detail`
//                * Look at all private/public accesses for exposed types
// TODO(dk949): See if any of the `requires` can be turned into `static_assert`s for better errors

#include <concepts>
#include <cstdlib>
#include <expected>
#include <format>
#include <functional>
#include <optional>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#if __cplusplus < 202'302L
#    error this file has to be compiled with at least C++23
#endif
#include <ut/static_string/static_string.hpp>


enum struct ParseMode { Short, Long, Positional };
enum struct ParseErrorKind { MissingRequiredError, MissingArgError, FormatErorr, ConfigError };

struct ParseError {
    ParseErrorKind kind;
    std::string message;
};

using ArgIterator = std::span<std::string_view>::iterator;

template<typename T>
struct NoVerifier {
    std::optional<ParseError> verify(T const &) {
        return std::nullopt;
    }
};

template<typename T>
struct SwitchParser {
    std::expected<T, ParseError> parse(
        ParseMode mode, int args_left, ArgIterator &current_argv, char short_mode_char, bool short_mode_last_in_group);
    std::optional<ParseError> verify(T const &);
};

template<>
struct SwitchParser<std::string_view> : NoVerifier<std::string_view> {
    std::expected<std::string_view, ParseError> parse(ParseMode mode,  //
        int args_left,
        ArgIterator &current_argv,
        char,
        bool short_mode_last_in_group) {
        switch (mode) {
            case ParseMode::Short:
                if (!short_mode_last_in_group) break;
                [[fallthrough]];
            case ParseMode::Long:
                if (args_left == 1) break;
                return *(current_argv++);
            case ParseMode::Positional: return *current_argv;
        }

        return std::unexpected {
            ParseError {ParseErrorKind::MissingArgError, std::format("Expected an argument for {}", *current_argv)}
        };
    }
};

template<>
struct SwitchParser<std::string> : SwitchParser<std::string_view> {
    std::expected<std::string, ParseError> parse(ParseMode mode,  //
        int args_left,
        ArgIterator &current_argv,
        char short_mode_char,
        bool short_mode_last_in_group) {
        return SwitchParser<std::string_view>::parse(mode, args_left, current_argv, short_mode_char, short_mode_last_in_group)
            .transform([](auto str) { return std::string {str}; });
    }
};

template<typename T>
struct SwitchParser<std::optional<T>> : SwitchParser<T> {
    std::expected<std::string, ParseError> parse(ParseMode mode,  //
        int args_left,
        ArgIterator &current_argv,
        char short_mode_char,
        bool short_mode_last_in_group) {
        return SwitchParser<T>::parse(mode, args_left, current_argv, short_mode_char, short_mode_last_in_group);
    }
};

class SwitchError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class ConfError : public SwitchError {
    using SwitchError::SwitchError;
};

template<typename... Ts>
struct all_unique : std::true_type { };

template<typename T>
struct all_unique<T> : std::true_type { };

template<typename T1, typename T2>
struct all_unique<T1, T2> : std::negation<std::is_same<T1, T2>> { };

template<typename T1, typename T2, typename... Rest>
struct all_unique<T1, T2, Rest...> : std::conjunction<all_unique<T1, Rest...>, all_unique<T2, Rest...>> { };

template<typename... Ts>
inline constexpr bool all_unique_v = all_unique<Ts...>::value;

template<typename T>
struct is_optional : std::false_type { };

template<typename T>
struct is_optional<std::optional<T>> : std::true_type { };

template<typename T>
inline constexpr bool is_optional_v = is_optional<T>::value;

struct ArgConf {
    bool operator==(ArgConf const &) const = default;
};

struct Help : ArgConf {
    std::string_view help;

    Help(std::string_view h)
            : help {h} { }
};

struct Metavar : ArgConf {
    std::string_view metavar;

    Metavar(std::string_view m)
            : metavar(m) { }

    bool operator==(Metavar const &) const = default;
};

template<typename T>
struct Default : ArgConf {
    T def;

    Default(T d)
            : def(std::move(d)) { }
};

struct RegEntryKey {
    char s_flag;
    std::string_view l_flag;
    Metavar metavar;
    bool operator==(RegEntryKey const &) const = default;
};

struct RegEntry {
    std::function<std::optional<ParseError>(ParseMode mode,  //
        int args_left,
        ArgIterator &current_argv,
        char short_mode_char,
        bool short_mode_last_in_group)>
        parse;
    std::function<std::optional<ParseError>()> verify;
    std::optional<Help> help;
};

template<>
struct std::hash<RegEntryKey> {
    std::size_t operator()(RegEntryKey const &key) const noexcept {
        return (std::hash<char> {}(key.s_flag) << 0)  //
             ^ (std::hash<std::string_view> {}(key.l_flag) << 1)
             ^ (std::hash<std::string_view> {}(key.metavar.metavar) << 2);
    }
};

using Registry = std::unordered_map<RegEntryKey, RegEntry>;

template<ut::StaticString name>
struct NthNameBase {
    static constexpr auto value = name;
};

template<std::size_t n, ut::StaticString... rest>
struct NthName {
    static constexpr ut::StaticString<1> value = "";
};

template<std::size_t n, ut::StaticString first, ut::StaticString... rest>
struct NthName<n, first, rest...> : std::conditional_t<n == 0, NthNameBase<first>, NthName<n - 1, rest...>> { };

template<std::size_t n, ut::StaticString... rest>
static constexpr auto nthName = NthName<n, rest...>::value;

struct RegistryHolderBase {
protected:
    static bool hasKey(RegEntryKey) {
        return false;
    }
};

template<typename Parser, typename ParentParserT = RegistryHolderBase>
struct RegistryHolder : ParentParserT {
    static inline Registry registry;
    using ThisParser = RegistryHolder<Parser, ParentParserT>;
    using ParentParser = ParentParserT;

    static bool hasKey(RegEntryKey key) {
        return registry.contains(key) || ParentParserT::hasKey(key);
    }

    static void addToRegistry(RegEntryKey key, RegEntry entry) {
        if (hasKey(key)) {
            bool positional = key.s_flag == 0 && key.l_flag.empty();
            throw ConfError(std::format("`{}{}{}{}` flag already added to registry",
                key.s_flag,
                !positional ? "/" : "",
                key.l_flag,
                positional ? "" : " ",
                key.metavar.metavar));
        }
        registry.insert({key, entry});
    }
};

template<typename T>
struct is_registry_holder : std::false_type { };

template<typename Parser, typename ParentParserT>
struct is_registry_holder<RegistryHolder<Parser, ParentParserT>> : std::true_type { };

template<typename T>
concept registry_holder = is_registry_holder<T>::value;

struct Positional { };

inline constexpr auto positional = Positional {};

template<typename T>
struct is_default : std::false_type { };

template<typename T>
struct is_default<Default<T>> : std::true_type { };

template<typename T>
inline constexpr bool is_default_v = is_default<T>::value;

template<typename T>
concept not_metavar = std::derived_from<T, ArgConf> && !std::same_as<T, Metavar>;

template<typename T, registry_holder ParentRegHolder>
struct Arg {
private:
    using Data =
        std::conditional_t<std::same_as<T, bool>, bool, std::conditional_t<is_optional_v<T>, T, std::optional<T>>>;
    char s_flag = 0;
    std::string_view l_flag = "";
    std::optional<Help> help;
    std::optional<Metavar> metavar;
    Data data {};
public:

    template<std::derived_from<ArgConf>... Confs>
    Arg(char sflag, std::string_view lflag, Confs &&...confs)
            : s_flag(sflag)
            , l_flag(lflag) {
        setConfs(std::forward<Confs>(confs)...);
        addToRegistry();
    }

    template<std::derived_from<ArgConf>... Confs>
    Arg(char sflag, Confs &&...confs)
            : Arg(sflag, "", std::forward<Confs>(confs)...) { }

    template<std::derived_from<ArgConf>... Confs>
    Arg(std::string_view lflag, Confs &&...confs)
            : Arg(0, lflag, std::forward<Confs>(confs)...) { }

    template<std::derived_from<ArgConf>... Confs>
    Arg(Positional, Confs &&...confs) requires(std::disjunction_v<std::is_same<Confs, Metavar>...>)
            : Arg(0, "", std::forward<Confs>(confs)...) { }

    operator T() {
        if constexpr (std::is_same_v<T, bool> || is_optional_v<T>)
            return data;
        else
            return data.value();
    }

    std::optional<ParseError> verify() {
        // TODO(dk949): custom verifier support
        if constexpr (std::is_same_v<T, bool>)
            return std::nullopt;
        else {
            SwitchParser<T> parser;

            if constexpr (is_optional_v<T>) {
                if (!data.has_value()) return std::nullopt;
                return parser.verify(*data);
            } else {
                if (!data.has_value()) return ParseError {ParseErrorKind::MissingRequiredError, std::format("")};
                return parser.verify(*data);
            }
        }
    }

    bool isPositional() {
        return s_flag == 0 && l_flag.empty();
    }
private:
    template<std::derived_from<ArgConf>... Confs>
    void setConfs(Confs &&...confs)
        requires(all_unique_v<Confs...> && (!std::same_as<T, bool> || (not_metavar<Confs> && ...))) {
        (void)([&]<typename Conf>(Conf &&conf) {
            using D = std::remove_cvref_t<Conf>;
            if constexpr (std::is_same_v<D, Help>)
                help = conf;
            else if constexpr (std::is_same_v<D, Metavar>)
                metavar = conf;
            else if constexpr (is_default_v<D>)
                data = conf.def;


            return true;
        }(confs) && ...);
    }

    void addToRegistry() {
        RegEntryKey key {s_flag, l_flag, metavar.value_or(Metavar {""})};
        RegEntry entry {
            .parse = [&](ParseMode mode,
                         int args_left,
                         ArgIterator &current_argv,
                         char short_mode_char,
                         bool short_mode_last_in_group) -> std::optional<ParseError> {
            if constexpr (std::is_same_v<T, bool>) {
                switch (mode) {
                    case ParseMode::Short:
                    case ParseMode::Long: data = true; return std::nullopt;
                    case ParseMode::Positional:
                        return ParseError {ParseErrorKind::ConfigError, "bool args cannot be positional"};
                    default:
                        return ParseError {ParseErrorKind::ConfigError,
                            std::format("Unexpected parse mode {}", std::to_underlying(mode))};
                }
            } else {
                SwitchParser<T> parser;
                if (auto val = parser.parse(mode, args_left, current_argv, short_mode_char, short_mode_last_in_group)) {
                    data = val.value();
                    return std::nullopt;
                } else
                    return val.error();
            }
        },
            .verify = [&]() { return verify(); },
            .help = help,
        };
        ParentRegHolder::addToRegistry(key, entry);
    }
};

#define UT_ARGS(name, ...) struct name : RegistryHolder<name __VA_OPT__(, __VA_ARGS__)>
#define UT_ARG(T)          static inline Arg<T, ThisParser>


#endif  // UT_SWITCHBOARD_HPP
