#ifndef UT_SWITCHBOARD_HPP
#define UT_SWITCHBOARD_HPP


#include <concepts>
#include <cstdlib>
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

struct RegEntryKey {
    std::string_view parser;
    char s_flag;
    std::string_view l_flag;
    bool operator==(RegEntryKey const &) const = default;
};

template<>
struct std::hash<RegEntryKey> {
    std::size_t operator()(RegEntryKey const &key) const noexcept {
        return (std::hash<std::string_view> {}(key.parser) << 0)  //
             ^ (std::hash<char> {}(key.s_flag) << 1)              //
             ^ (std::hash<std::string_view> {}(key.l_flag) << 2);
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

template<ut::StaticString ThisParserName, typename ParentParserT = ArgParserBase>
struct RegistryHolder {
    static Registry registry;
    using ThisParser = RegistryHolder<ThisParserName, ParentParserT>;
    using ParentParser = ParentParserT;
};

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
concept not_metavar = std::derived_from<T, ArgConf> && !std::same_as<T, Metavar>;

template<typename T, typename ParentArgs>
struct Arg {
private:
    char s_flag = 0;
    std::string_view l_flag = "";
    std::optional<Help> help;
    std::optional<Metavar> metavar;
    T data {};
public:

    template<std::derived_from<ArgConf>... Confs>
    Arg(char sflag, std::string_view lflag, Confs &&...confs)
            : s_flag(sflag)
            , l_flag(lflag) {
        setConfs(std::forward<Confs>(confs)...);
    }

    template<std::derived_from<ArgConf>... Confs>
    Arg(char sflag, Confs &&...confs)
            : Arg(sflag, "", std::forward<Confs>(confs)...) { }

    template<std::derived_from<ArgConf>... Confs>
    Arg(std::string_view lflag, Confs &&...confs)
            : Arg(0, lflag, std::forward<Confs>(confs)...) { }

    operator T() {
        return data;
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

            return true;
        }(confs) && ...);
    }
};

#define UT_ARGS(name, ...) struct name : private RegistryHolder<#name __VA_OPT__(, RegistryHolder<#__VA_ARGS__>)>
#define UT_ARG(T)          Arg<T, ThisParser>


#endif  // UT_SWITCHBOARD_HPP
