#ifndef UT_SWITCHBOARD_HPP
#define UT_SWITCHBOARD_HPP


#include <cstdlib>
#include <optional>
#include <type_traits>
#if __cplusplus < 202'002L
#    error this file has to be compiled with at least C++20
#endif
#include <ut/static_string/static_string.hpp>

namespace ut::sw {

template<StaticString str>
struct Name {
    static constexpr auto name = str;
};

template<StaticString str>
constexpr auto name = Name<str> {};

namespace detail {

    template<std::size_t str_size, std::size_t name_size>
    struct Conf {
        StaticString<str_size> str = "";
        StaticString<name_size> name = "";

        template<std::size_t S>
        constexpr Conf(char const (&s)[S])
                : str(s) { }

        template<StaticString s>
        constexpr Conf(Name<s> n)
                : name(n.name) { }
    };

    template<std::size_t S>
    Conf(char const (&)[S]) -> Conf<S, 1>;

    template<StaticString s>
    Conf(Name<s> n) -> Conf<1, s.size()>;

    template<Conf conf>
    struct NthConfBase {
        static constexpr auto value = conf;
    };

    template<std::size_t n, Conf... rest>
    struct NthConf {
        static_assert(sizeof...(rest), "Too few configuration parameters");
    };

    template<std::size_t n, Conf first, Conf... rest>
    struct NthConf<n, first, rest...> : std::conditional_t<n == 0, NthConfBase<first>, NthConf<n - 1, rest...>> { };

    template<std::size_t n, Conf... rest>
    static constexpr auto nthConf = NthConf<n, rest...>::value;

#define UT_SW_DETAIL_ASSERT(b, msg, when) \
    static_assert((b),                    \
        "\n\n"                            \
        "SwitchBoard error!\n"            \
        "\n" msg "\n"                     \
        "This error occurred when " when "\n\n")


}  // namespace detail

template<typename T, detail::Conf... conf>
struct Arg {
    using type = T;
    std::optional<std::string_view> short_flag;
    std::optional<std::string_view> long_flag;
    std::string_view name;

    consteval Arg() {
        constexpr auto maybe_short = detail::nthConf<0, conf...>.str;
        UT_SW_DETAIL_ASSERT(!maybe_short.empty(), "First argument must be a non-empty string", "defining an `Arg`");
        constexpr auto short_res = getShort();
        short_flag = short_res.first;
        // constexpr std::string_view maybe_long = detail::nthConf<0, conf...>.str.view();
        // UT_SW_DETAIL_ASSERT(!maybe_short.empty(), "First argument must be a non-empty string", "defining an `Arg`");
    };
private:
    constexpr std::pair<std::optional<std::string_view>, std::size_t> getShort() {
        return std::pair {std::nullopt, 0u};
    }
};

template<typename A>
struct is_arg : std::false_type { };

template<typename T, detail::Conf... def>
struct is_arg<Arg<T, def...>> : std::true_type { };

template<typename T, detail::Conf... defs>
static constexpr auto arg = Arg<T, defs...> {};

template<typename... T>
struct ArgParser {
    static_assert((is_arg<T>::value && ...));

    consteval ArgParser() {
        (T {}, ...);
    }
};

}  // namespace ut::sw

#endif  // UT_SWITCHBOARD_HPP
