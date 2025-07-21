#ifndef UT_SWITCHBOARD_HPP
#define UT_SWITCHBOARD_HPP

#include <concepts>
#include <string_view>
#include <type_traits>
#include <vector>

namespace ut::sw {

class SwitchError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class ConfError : public SwitchError {
    using SwitchError::SwitchError;
};

namespace detail {
    struct ArgConf { };
}

// TODO(dk949): Add Name() Conf to not rely on Metavar when only short flag is supplied

struct Help : detail::ArgConf {
    std::string help;

    Help(std::string h)
            : help(std::move(h)) { }
};

struct Metavar : detail::ArgConf {
    std::string_view metavar;

    Metavar(std::string_view m)
            : metavar(m) { }
};

struct Default : detail::ArgConf {
    std::string def;

    Default(std::string d)
            : def(std::move(d)) { }
};

struct Optional : detail::ArgConf {
    operator bool() {
        return true;
    }
};

inline constexpr auto optional = Optional {};

namespace detail {
    enum struct ArgKind { Argument, Flag, Positional };

    struct Arg {
        ArgKind kind;
        char short_switch;
        std::string_view long_switch;
        std::string_view name;
        std::string metavar;
        std::string help;
        std::string def;
        bool is_optional;
    };

    template<typename... Ts>
    struct all_unique;

    template<>
    struct all_unique<> : std::true_type { };

    template<typename T>
    struct all_unique<T> : std::true_type { };

    template<typename T1, typename T2>
    struct all_unique<T1, T2> : std::negation<std::is_same<T1, T2>> { };

    template<typename T1, typename T2, typename... Rest>
    struct all_unique<T1, T2, Rest...>
            : std::conjunction<all_unique<T1, T2>, all_unique<T1, Rest...>, all_unique<T2, Rest...>> { };

    template<typename... Ts>
    inline constexpr bool all_unique_v = all_unique<Ts...>::value;

    static_assert(all_unique_v<int, float, char>);
    static_assert(!all_unique_v<int, float, int, char>);

    template<typename... Ts>
    struct has_metevar : std::disjunction<std::is_same<Metavar, Ts>...> { };

    template<typename... Ts>
    inline constexpr bool has_metevar_v = has_metevar<Ts...>::value;
    static_assert(!has_metevar_v<int, float, char>);
    static_assert(has_metevar_v<int, float, char, Metavar>);

}  // namespace detail

class ArgumentParser {
    std::vector<detail::Arg> m_args;
public:
    template<std::derived_from<detail::ArgConf>... Confs>
    ArgumentParser &addArg(char short_switch, std::string_view long_switch, Confs... confs)
        requires(detail::all_unique_v<Confs...>) {
        addArgOrFlag(detail::ArgKind::Argument, short_switch, long_switch, confs...);
        return *this;
    }

    template<std::derived_from<detail::ArgConf>... Confs>
    ArgumentParser &addArg(char short_switch, Confs... confs)
        requires(detail::all_unique_v<Confs...> &&detail::has_metevar_v<Confs...>) {
        addArgOrFlag(detail::ArgKind::Argument, short_switch, "", confs...);
        return *this;
    }

    template<std::derived_from<detail::ArgConf>... Confs>
    ArgumentParser &addArg(std::string_view long_switch, Confs... confs) requires(detail::all_unique_v<Confs...>) {
        addArgOrFlag(detail::ArgKind::Argument, 0, long_switch, confs...);
        return *this;
    }

    template<std::derived_from<detail::ArgConf>... Confs>
    ArgumentParser &addFlag(char short_switch, std::string_view long_switch, Confs... confs)
        requires(detail::all_unique_v<Confs...>) {
        addArgOrFlag(detail::ArgKind::Flag, short_switch, long_switch, confs...);
        return *this;
    }

    template<std::derived_from<detail::ArgConf>... Confs>
    ArgumentParser &addFlag(char short_switch, Confs... confs)
        requires(detail::all_unique_v<Confs...> &&detail::has_metevar_v<Confs...>) {
        addArgOrFlag(detail::ArgKind::Flag, short_switch, "", confs...);
        return *this;
    }

    template<std::derived_from<detail::ArgConf>... Confs>
    ArgumentParser &addFlag(std::string_view long_switch, Confs... confs) requires(detail::all_unique_v<Confs...>) {
        addArgOrFlag(detail::ArgKind::Flag, 0, long_switch, confs...);
        return *this;
    }

    template<std::derived_from<detail::ArgConf>... Confs>
    ArgumentParser &addPositional(std::string_view name, Confs... confs) requires(detail::all_unique_v<Confs...>) {
        m_args.push_back(detail::Arg {
            .kind = detail::ArgKind::Positional,
            .short_switch = 0,
            .long_switch = "",
            .name = name,
            .metavar = getConf<std::string_view, Metavar>(confs...),
            .help = getConf<Help>(confs...),
            .def = getConf<std::string, Default>(confs...),
            .is_optional = getConf<bool, Optional>(confs...),
        });
        return *this;
    }
private:
    template<typename T, std::derived_from<detail::ArgConf> Conf, bool move = true, std::derived_from<detail::ArgConf>... Confs>
    std::string getConf(Confs &&...confs) {
        T conf {};
        ([&]() {
            using D = std::remove_cvref_t<Confs>;
            if constexpr (std::is_same_v<D, Conf>) {
                if (move)
                    conf = std::move(confs);
                else
                    conf = confs;
                return false;
            }
            return true;
        }() && ...);
        return conf;
    }

    template<std::derived_from<detail::ArgConf>... Confs>
    void addArgOrFlag(detail::ArgKind kind, char short_switch, std::string_view long_switch, Confs &&...confs) {
        if (long_switch == "--") {
            throw ConfError("Long switch cannot have the value --");
        }
        std::string_view name = [&] {
            if (!long_switch.empty()) return long_switch;
            return getConf<std::string_view, Metavar, false>(std::forward<Confs>(confs)...);
        }();
        m_args.push_back(detail::Arg {
            .kind = kind,
            .short_switch = short_switch,
            .long_switch = long_switch,
            .name = name,
            .metavar = getConf<std::string_view, Metavar>(confs...),
            .help = getConf<Help>(confs...),
            .def = getConf<std::string, Default>(confs...),
            .is_optional = getConf<bool, Optional>(confs...),
        });
    }
};

}  // namespace ut::sw

#endif  // UT_SWITCHBOARD_HPP
