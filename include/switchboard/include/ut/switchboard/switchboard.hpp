#ifndef UT_SWITCHBOARD_HPP
#define UT_SWITCHBOARD_HPP


#include <cstdlib>
#include <optional>
#include <type_traits>
#if __cplusplus < 202'302L
#    error this file has to be compiled with at least C++23
#endif
#include <ut/static_string/static_string.hpp>

namespace ut::sw {

template<StaticString str>
struct Name {
    static constexpr auto name = str;
};

template<StaticString str>
struct Desc {
    static constexpr auto desc = str;
};

template<StaticString str>
constexpr auto name = Name<str> {};

template<StaticString str>
constexpr auto desc = Desc<str> {};

namespace detail {

    template<std::size_t str_size, std::size_t name_size, std::size_t desc_size>
    struct Conf {
        StaticString<str_size> str = "";
        StaticString<name_size> name = "";
        StaticString<desc_size> desc = "";

        template<std::size_t S>
        constexpr Conf(char const (&s)[S])
                : str(s) { }

        template<StaticString s>
        constexpr Conf(Name<s> n)
                : name(n.name) { }

        template<StaticString s>
        constexpr Conf(Desc<s> d)
                : desc(d.desc) { }

        constexpr bool empty() const {
            return str.empty() && name.empty() && desc.empty();
        }
    };

    template<std::size_t S>
    Conf(char const (&)[S]) -> Conf<S, 1, 1>;

    template<StaticString s>
    Conf(Name<s> n) -> Conf<1, s.size(), 1>;

    template<StaticString s>
    Conf(Desc<s> n) -> Conf<1, 1, s.size()>;

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

#define UT_SW_DETAIL_MAKE_MESSAGE(msg, when) \
    "\n\n"                                   \
    "SwitchBoard error!\n"                   \
    "\n" msg "\n"                            \
    "This error occurred when " when "\n\n"
#define UT_SW_DETAIL_STATIC_ASSERT(b, msg, when) static_assert((b), UT_SW_DETAIL_MAKE_MESSAGE(msg, when))
#define UT_SW_DETAIL_ARG_ASSERT(b, msg, pos)                                               \
    do {                                                                                   \
        if constexpr (pos == 0) {                                                          \
            UT_SW_DETAIL_STATIC_ASSERT(b, msg, "defining an `Arg` (1st config option).");  \
        } else if constexpr (pos == 1) {                                                   \
            UT_SW_DETAIL_STATIC_ASSERT(b, msg, "defining an `Arg` (2nd config option).");  \
        } else if constexpr (pos == 2) {                                                   \
            UT_SW_DETAIL_STATIC_ASSERT(b, msg, "defining an `Arg` (3rd config option).");  \
        } else if constexpr (pos == 3) {                                                   \
            UT_SW_DETAIL_STATIC_ASSERT(b, msg, "defining an `Arg` (4th config option).");  \
        } else if constexpr (pos == 4) {                                                   \
            UT_SW_DETAIL_STATIC_ASSERT(b, msg, "defining an `Arg` (5th config option).");  \
        } else if constexpr (pos == 5) {                                                   \
            UT_SW_DETAIL_STATIC_ASSERT(b, msg, "defining an `Arg` (6th config option).");  \
        } else if constexpr (pos == 6) {                                                   \
            UT_SW_DETAIL_STATIC_ASSERT(b, msg, "defining an `Arg` (7th config option).");  \
        } else if constexpr (pos == 7) {                                                   \
            UT_SW_DETAIL_STATIC_ASSERT(b, msg, "defining an `Arg` (8th config option).");  \
        } else if constexpr (pos == 8) {                                                   \
            UT_SW_DETAIL_STATIC_ASSERT(b, msg, "defining an `Arg` (9th config option).");  \
        } else if constexpr (pos == 9) {                                                   \
            UT_SW_DETAIL_STATIC_ASSERT(b, msg, "defining an `Arg` (10th config option)."); \
        } else {                                                                           \
            UT_SW_DETAIL_STATIC_ASSERT(false,                                              \
                "(internal) Too many config options for `Arg`",                            \
                "handling Conf position printing in static_assert.");                      \
        }                                                                                  \
    } while (0)

}  // namespace detail

template<typename T, detail::Conf... conf>
struct Arg {
    using type = T;
    static constexpr auto out_of_args = ~std::size_t(0);
    std::optional<std::string_view> short_flag = std::nullopt;
    std::optional<std::string_view> long_flag = std::nullopt;
    std::optional<std::string_view> name = std::nullopt;
    std::optional<std::string_view> description = std::nullopt;

    consteval Arg() {
        constexpr auto short_res = getShort<0>();
        short_flag = short_res.first;
        constexpr auto long_res = getLong<short_res.second>();
        long_flag = long_res.first;
        parseRest<long_res.second, long_res.first.has_value()>();

        UT_SW_DETAIL_STATIC_ASSERT(long_res.first || short_res.first,
            "Either a long (e.g. --flag) or short (e.g. -f) flag must be provided.",
            "defining an `Arg`");
        if (!name) name = long_flag.value().substr(2);
    };
private:
    template<std::size_t pos>
    static constexpr std::pair<std::optional<std::string_view>, std::size_t> getShort() {
        if constexpr (pos >= sizeof...(conf))
            return std::pair {std::nullopt, out_of_args};
        else {
            static constexpr auto maybe_short = detail::nthConf<pos, conf...>;
            UT_SW_DETAIL_ARG_ASSERT(!maybe_short.empty(), "Expected all strings to be non-empty", pos);
            if constexpr (maybe_short.str.size() == 2 && maybe_short.str.starts_with('-'))
                return std::pair {maybe_short.str.view(), std::size_t(pos + 1)};
            return std::pair {std::nullopt, std::size_t(pos)};
        }
    }

    template<std::size_t pos>
    static constexpr std::pair<std::optional<std::string_view>, std::size_t> getLong() {
        if constexpr (pos >= sizeof...(conf))
            return std::pair {std::nullopt, out_of_args};
        else {
            static constexpr auto maybe_long = detail::nthConf<pos, conf...>;
            UT_SW_DETAIL_ARG_ASSERT(!maybe_long.empty(), "Expected all strings to be non-empty", pos);
            if constexpr (maybe_long.str.size() > 2 && maybe_long.str.starts_with("--"))
                return std::pair {maybe_long.str.view(), std::size_t(pos + 1)};

            return std::pair {std::nullopt, std::size_t(pos)};
        }
    }

    template<std::size_t pos>
    static constexpr std::pair<std::optional<std::string_view>, std::size_t> getName() {
        if constexpr (pos >= sizeof...(conf))
            return std::pair {std::nullopt, out_of_args};
        else {
            static constexpr auto maybe_name = detail::nthConf<pos, conf...>;
            UT_SW_DETAIL_ARG_ASSERT(!maybe_name.empty(), "Expected all strings to be non-empty", pos);
            if constexpr (!maybe_name.name.empty()) return std::pair {maybe_name.name.view(), pos + 1};

            return std::pair {std::nullopt, pos};
        }
    }

    template<std::size_t pos>
    static constexpr std::pair<std::optional<std::string_view>, std::size_t> getDesc() {
        if constexpr (pos >= sizeof...(conf))
            return std::pair {std::nullopt, out_of_args};
        else {
            static constexpr auto maybe_desc = detail::nthConf<pos, conf...>;
            UT_SW_DETAIL_ARG_ASSERT(!maybe_desc.empty(), "Expected all strings to be non-empty", pos);
            if constexpr (!maybe_desc.desc.empty()) return std::pair {maybe_desc.desc.view(), pos + 1};

            return std::pair {std::nullopt, pos};
        }
    }

    template<std::size_t pos, bool has_long, bool has_name = false>
    constexpr void parseRest() {
        if constexpr (pos >= sizeof...(conf)) {
            UT_SW_DETAIL_STATIC_ASSERT(has_long || has_name,
                "Either a long flag (e.g. --flag) or a `name<...>` must be provided.",
                "defining an `Arg`");
            // if(!description) throw 1;
            return;
        } else {
            constexpr auto name_res = getName<pos>();
            if constexpr (name_res.first) name = name_res.first;
            constexpr auto desc_res = getDesc<name_res.second>();
            if constexpr (desc_res.first) description = desc_res.first;
            UT_SW_DETAIL_ARG_ASSERT(name_res.first || desc_res.first, "Could not parse argument", pos);
            return parseRest<desc_res.second, has_long, has_name || name_res.first.has_value()>();
        }
    }
};

namespace detail {
    template<typename A>
    struct is_arg : std::false_type { };

    template<typename T, detail::Conf... def>
    struct is_arg<Arg<T, def...>> : std::true_type { };
}

template<typename T, detail::Conf... defs>
static constexpr auto arg = Arg<T, defs...> {};

template<typename... T>
struct ArgParser {
    static_assert((detail::is_arg<T>::value && ...));
    std::tuple<T...> args;

    consteval ArgParser()
            : args(T {}...) { }
};

}  // namespace ut::sw

#endif  // UT_SWITCHBOARD_HPP
