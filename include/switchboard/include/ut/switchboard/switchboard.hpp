#ifndef UT_SWITCHBOARD_HPP
#define UT_SWITCHBOARD_HPP


#include <concepts>
#include <cstdlib>
#include <functional>
#include <optional>
#include <stdexcept>
#include <type_traits>
#if __cplusplus < 202'302L
#    error this file has to be compiled with at least C++23
#endif
#include <ut/static_string/static_string.hpp>

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

struct RegEntryKey {
    char s_flag;
    std::string_view l_flag;
    bool operator==(RegEntryKey const &) const = default;
};

enum struct ParseMode { Short, Long, Positional };
enum struct ParseErrorKind { OutOfArgsErorr, FormatErorr, Internal };

struct ParseError {
    ParseErrorKind kind;
    std::string message;
};

struct RegEntry {
    std::function<std::optional<ParseError>(ParseMode mode,  //
        int total_argc,
        int &current_argc,
        char **&current_argv,
        char short_mode_char,
        bool short_mode_last_in_group)>
        parse;
    std::function<bool()> verify;
};

template<>
struct std::hash<RegEntryKey> {
    std::size_t operator()(RegEntryKey const &key) const noexcept {
        return (std::hash<char> {}(key.s_flag) << 0)  //
             ^ (std::hash<std::string_view> {}(key.l_flag) << 1);
    }
};

struct Registry { };

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

struct ArgParserBase { };

template<typename Parser, typename ParentParserT = ArgParserBase>
struct RegistryHolder : ParentParserT {
    static Registry registry;
    using ThisParser = RegistryHolder<Parser, ParentParserT>;
    using ParentParser = ParentParserT;

    static void addToRegistry(RegEntryKey key, RegEntry entry) { }
};

template<typename T>
struct is_registry_holder : std::false_type { };

template<typename Parser, typename ParentParserT>
struct is_registry_holder<RegistryHolder<Parser, ParentParserT>> : std::true_type { };

template<typename T>
concept registry_holder = is_registry_holder<T>::value;

struct ArgConf { };

struct Help : ArgConf {
    std::string_view help;

    Help(std::string_view h)
            : help {h} { }
};

struct Metavar : ArgConf {
    std::string_view metavar;

    Metavar(std::string_view m)
            : metavar(m) { }
};

template<typename T>
struct Default : ArgConf {
    T def;

    Default(T d)
            : def(std::move(d)) { }
};

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
    Arg(Positional, Confs &&...confs)
            : Arg(0, "", std::forward<Confs>(confs)...) { }

    operator T() {
        if constexpr (std::is_same_v<T, bool> || is_optional_v<T>)
            return data;
        else
            return data.value();
    }

    bool verify() {
        // TODO(dk949): custom verifier support
        if constexpr (std::is_same_v<T, bool> || is_optional_v<T>) {
            return true;
        } else {
            return data.has_value();
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
        RegEntryKey key {s_flag, l_flag};
        RegEntry entry {.parse = [&](ParseMode mode,
                                     int total_argc,
                                     int &current_argc,
                                     char **&current_argv,
                                     char short_mode_char,
                                     bool short_mode_last_in_group) -> std::optional<ParseError> {
            if constexpr (std::is_same_v<T, bool>) {
                switch (mode) {
                    case ParseMode::Short:
                    case ParseMode::Long: data = true; return std::nullopt;
                    case ParseMode::Positional:
                        return ParseError {ParseErrorKind::Internal, "bool args cannot be positional"};
                }
            }
            return ParseError {ParseErrorKind::Internal, "not implemented"};
        },
            .verify = [&]() {
            return verify();
        }};
        ParentRegHolder::addToRegistry(key, entry);
    }
};

#define UT_ARGS(name, ...) struct name : RegistryHolder<name __VA_OPT__(, __VA_ARGS__)>
#define UT_ARG(T)          static inline Arg<T, ThisParser>


#endif  // UT_SWITCHBOARD_HPP
