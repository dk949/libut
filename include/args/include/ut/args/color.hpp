#ifndef UT_COLOR_HPP
#define UT_COLOR_HPP

#include <concepts>
#include <cstdint>
#include <format>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#ifdef _WIN32
#    include <io.h>

#    include <cstdio>
#else
#    include <unistd.h>
#endif

namespace ut {
struct Color {
private:
    struct Runtime { };

    struct Monostate { };
    enum struct ColKind : bool { Fg, Bg };

    enum struct TermColor : std::uint8_t {
        Black,
        Red,
        Green,
        Yellow,
        Blue,
        Magenta,
        Cyan,
        White,
        _256Color,
        Rgb,
    };

public:
    struct ColoriserBase { };

private:
    template<auto C, typename T, ColKind Kind>
    struct Coloriser : Color::ColoriserBase {
    private:
        static_assert(std::same_as<decltype(C), Color::Runtime> || std::same_as<decltype(C), Color>);
        static constexpr auto const_col = C;
        static constexpr bool is_runtime = std::same_as<decltype(C), Color::Runtime>;
        static constexpr ColKind col_kind = Kind;
        using RuntimeColorT = std::conditional_t<is_runtime, Color, Monostate>;

        T const *m_value;
        [[no_unique_address]]
        RuntimeColorT m_col;
    public:

        constexpr explicit Coloriser(T const *value, Color color) requires(is_runtime)
                : m_value(value)
                , m_col(color) { }

        constexpr explicit Coloriser(T const *value) requires(!is_runtime)
                : m_value(value)
                , m_col() { }

        [[nodiscard]]
        constexpr T const &getValue() const noexcept {
            return *m_value;
        }

        [[nodiscard]]
        constexpr Color getColor() const noexcept {
            if constexpr (is_runtime) {
                return m_col;
            } else {
                return const_col;
            }
        }

        [[nodiscard]]
        static constexpr bool isFg() noexcept {
            return col_kind == ColKind::Fg;
        }

        template<typename U, typename Ctx>
        friend struct std::formatter;  // NOLINT(cert-dcl58-cpp) -- required to grant formatter access to private members
    };

    template<auto C, typename T>
    friend constexpr Coloriser<C, T, ColKind::Fg> fg(T const &);

    template<typename T>
    friend constexpr Coloriser<Runtime {}, T, ColKind::Fg> fg(T const &, Color);

    template<auto C, typename T>
    friend constexpr Coloriser<C, T, ColKind::Bg> bg(T const &);

    template<typename T>
    friend constexpr Coloriser<Runtime {}, T, ColKind::Bg> bg(T const &, Color);
public:
    TermColor m_term;
    std::uint8_t m_r;
    std::uint8_t m_g;
    std::uint8_t m_b;

    constexpr Color(std::uint8_t r, std::uint8_t g, std::uint8_t b)  // NOLINT(bugprone-easily-swappable-parameters) --
                                                                     // r/g/b are semantically distinct colour channels
            : m_term(TermColor::Rgb)
            , m_r(r)
            , m_g(g)
            , m_b(b) { }

    constexpr explicit Color(std::uint8_t id)
            : m_term(TermColor::_256Color)
            , m_r(id)
            , m_g()
            , m_b() { }

private:
    constexpr explicit Color(TermColor col)
            : m_term(col)
            , m_r()
            , m_g()
            , m_b() { }
public:
    static Color const Black;
    static Color const Red;
    static Color const Green;
    static Color const Yellow;
    static Color const Blue;
    static Color const Magenta;
    static Color const Cyan;
    static Color const White;

    [[nodiscard]]
    std::string ansiEscape(bool isFg) const {
        auto const base = isFg ? 30u : 40u;
        static constexpr auto fg_code = 38u;
        static constexpr auto bg_code = 48u;
        switch (m_term) {
            using enum TermColor;
            case Black:
            case Red:
            case Green:
            case Yellow:
            case Blue:
            case Magenta:
            case Cyan:
            case White: return std::format("\033[{}m", base + std::to_underlying(m_term));
            case _256Color: return std::format("\033[{};5;{}m", isFg ? fg_code : bg_code, m_r);
            case Rgb: return std::format("\033[{};2;{};{};{}m", isFg ? fg_code : bg_code, m_r, m_g, m_b);
            default: std::unreachable();
        }
    }
};

inline constexpr Color Color::Black {Color::TermColor::Black};
inline constexpr Color Color::Red {Color::TermColor::Red};
inline constexpr Color Color::Green {Color::TermColor::Green};
inline constexpr Color Color::Yellow {Color::TermColor::Yellow};
inline constexpr Color Color::Blue {Color::TermColor::Blue};
inline constexpr Color Color::Magenta {Color::TermColor::Magenta};
inline constexpr Color Color::Cyan {Color::TermColor::Cyan};
inline constexpr Color Color::White {Color::TermColor::White};

namespace detail {
    inline thread_local std::optional<bool> ttyOverride;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
                                                          // -- RAII guard mutates this intentionally

    struct TtyOverrideGuard {
    private:
        std::optional<bool> m_old_value;
    public:
        explicit TtyOverrideGuard(bool value) noexcept
                : m_old_value(ttyOverride) {
            ttyOverride = value;
        }

        TtyOverrideGuard(TtyOverrideGuard const &) = delete;
        TtyOverrideGuard &operator=(TtyOverrideGuard const &) = delete;
        TtyOverrideGuard(TtyOverrideGuard &&) = delete;
        TtyOverrideGuard &operator=(TtyOverrideGuard &&) = delete;

        ~TtyOverrideGuard() noexcept {
            ttyOverride = m_old_value;
        }
    };

    [[nodiscard]]
    inline bool isStdoutTty() noexcept {
        if (ttyOverride.has_value()) {
            return *ttyOverride;
        }
        static bool const result = []() noexcept -> bool {
#ifdef _WIN32
            return _isatty(_fileno(stdout)) != 0;
#else
            return isatty(STDOUT_FILENO) != 0;
#endif
        }();
        return result;
    }
}  // namespace detail

template<auto C, typename T>
constexpr Color::Coloriser<C, T, Color::ColKind::Fg> fg(T const &v) {
    return Color::Coloriser<C, T, Color::ColKind::Fg>(&v);
}

template<typename T>
constexpr Color::Coloriser<Color::Runtime {}, T, Color::ColKind::Fg> fg(T const &v, Color col) {
    return Color::Coloriser<Color::Runtime {}, T, Color::ColKind::Fg> {&v, col};
}

template<auto C, typename T>
constexpr Color::Coloriser<C, T, Color::ColKind::Bg> bg(T const &v) {
    return Color::Coloriser<C, T, Color::ColKind::Bg>(&v);
}

template<typename T>
constexpr Color::Coloriser<Color::Runtime {}, T, Color::ColKind::Bg> bg(T const &v, Color col) {
    return Color::Coloriser<Color::Runtime {}, T, Color::ColKind::Bg> {&v, col};
}

}  // namespace ut

template<typename Coloriser, typename CharT>
requires std::derived_from<Coloriser, ut::Color::ColoriserBase> &&requires(Coloriser const &c) {
    c.getValue();
    {c.getColor()}->std::same_as<ut::Color>;
    {Coloriser::isFg()}->std::same_as<bool>;
}

struct std::formatter<Coloriser, CharT> {  // NOLINT(cert-dcl58-cpp) -- std::formatter specialisation; permitted per
                                           // [namespace.std]
private:
    using ValueT = std::remove_cvref_t<decltype(std::declval<Coloriser const &>().getValue())>;
    std::formatter<ValueT, CharT> m_inner;

public:
    template<typename ParseContext>
    constexpr auto parse(ParseContext &ctx) {
        return m_inner.parse(ctx);
    }

    template<typename FormatContext>
    auto format(Coloriser const &col, FormatContext &ctx) const {
        bool const tty = ut::detail::isStdoutTty();
        auto out = ctx.out();
        if (tty) {
            out = std::format_to(out, "{}", col.getColor().ansiEscape(Coloriser::isFg()));
            ctx.advance_to(out);
        }
        out = m_inner.format(col.getValue(), ctx);
        if (tty) {
            out = std::format_to(out, "\033[0m");
        }
        return out;
    }
};

#endif  // UT_COLOR_HPP
