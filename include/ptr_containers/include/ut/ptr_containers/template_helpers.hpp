#ifndef CONCEPTS_HPP
#define CONCEPTS_HPP
#include <concepts>
#include <memory>

namespace ut::detail {
template<typename A, typename B>
concept SameAsRemoveCVRef = std::same_as<std::remove_cvref_t<A>, std::remove_cvref_t<B>>;

template<typename D, typename B>
concept DerivedOrEqualTo = SameAsRemoveCVRef<D, B> || std::derived_from<std::remove_cvref_t<D>, std::remove_cvref_t<B>>;

template<typename T>
struct AddTransitiveConst;

template<typename T>
requires(!std::is_pointer_v<T>) struct AddTransitiveConst<T> {
    using type = std::add_const_t<T>;
};

template<typename T>
requires(std::is_pointer_v<T>) struct AddTransitiveConst<T> {
public:
    using type = std::add_const_t<std::add_pointer_t<typename AddTransitiveConst<std::remove_pointer_t<T>>::type>>;
};

template<typename T>
using AddTransitiveConstT = AddTransitiveConst<T>::type;

template<typename T>
struct IsUniquePtr : std::false_type { };

template<typename T>
struct IsUniquePtr<std::unique_ptr<T>> : std::true_type { };

template<typename T>
inline constexpr bool IsUniquePtrV = IsUniquePtr<T>::value;

template<typename T>
concept UniquePtr = IsUniquePtrV<T>;

template<typename T>
concept NotUniquePtr = !IsUniquePtrV<T>;

template<typename T>
concept Pointer = std::is_pointer_v<T>;

[[nodiscard]]
static constexpr auto distance(auto a, auto b) {
    return std::distance(AddTransitiveConstT<decltype(a)>(a), AddTransitiveConstT<decltype(b)>(b));
}

template<typename A, typename B>
struct PtrCmp {
    constexpr bool operator()(A const *, B const *) const {
        return false;
    }
};

template<typename T>
struct PtrCmp<T, T> {
    constexpr bool operator()(T const *a, T const *b) const {
        return a == b;
    }
};

template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

#define UT_REQUIRE_UNARY(expr) \
    requires(T t) {            \
        expr;                  \
    }

#define UT_REQUIRE_BINARY(expr) \
    requires(T a, U b) {        \
        expr;                   \
    }

template<typename T>
concept HasUnaryPlus = UT_REQUIRE_UNARY(+t);
template<typename T, typename U>
concept HasBinaryPlus = UT_REQUIRE_BINARY(a + b);
template<typename T>
concept HasPrefixPlus = UT_REQUIRE_UNARY(++t);
template<typename T>
concept HasPostfixPlus = UT_REQUIRE_UNARY(t++);
template<typename T, typename U>
concept HasPlusEqual = UT_REQUIRE_BINARY(a += b);


template<typename T>
concept HasUnaryMinus = UT_REQUIRE_UNARY(-t);
template<typename T, typename U>
concept HasBinaryMinus = UT_REQUIRE_BINARY(a - b);
template<typename T>
concept HasPrefixMinus = UT_REQUIRE_UNARY(--t);
template<typename T>
concept HasPostfixMinus = UT_REQUIRE_UNARY(t--);
template<typename T, typename U>
concept HasMinusEqual = UT_REQUIRE_BINARY(a -= b);


template<typename T, typename U>
concept HasMult = UT_REQUIRE_BINARY(a * b);
template<typename T, typename U>
concept HasMultEqual = UT_REQUIRE_BINARY(a *= b);


template<typename T, typename U>
concept HasDiv = UT_REQUIRE_BINARY(a * b);
template<typename T, typename U>
concept HasDivEqual = UT_REQUIRE_BINARY(a * b);


template<typename T, typename U>
concept HasPercent = UT_REQUIRE_BINARY(a % b);
template<typename T, typename U>
concept HasPercentEqual = UT_REQUIRE_BINARY(a %= b);


template<typename T, typename U>
concept HasXor = UT_REQUIRE_BINARY(a ^ b);
template<typename T, typename U>
concept HasXorEqual = UT_REQUIRE_BINARY(a ^ b);


template<typename T, typename U>
concept HasBand = UT_REQUIRE_BINARY(a & b);
template<typename T, typename U>
concept HasBandEqual = UT_REQUIRE_BINARY(a &= b);
template<typename T, typename U>
concept HasLand = UT_REQUIRE_BINARY(a && b);


template<typename T, typename U>
concept HasBor = UT_REQUIRE_BINARY(a | b);
template<typename T, typename U>
concept HasBorEqual = UT_REQUIRE_BINARY(a |= b);
template<typename T, typename U>
concept HasLor = UT_REQUIRE_BINARY(a || b);


template<typename T>
concept HasBnot = UT_REQUIRE_UNARY(~t);


template<typename T>
concept HasLnot = UT_REQUIRE_UNARY(!t);


template<typename T, typename U>
concept HasLshift = UT_REQUIRE_BINARY(a << b);
template<typename T, typename U>
concept HasLshiftEqual = UT_REQUIRE_BINARY(a << b);


template<typename T, typename U>
concept HasRshift = UT_REQUIRE_BINARY(a >> b);
template<typename T, typename U>
concept HasRshiftEqual = UT_REQUIRE_BINARY(a >>= b);


template<typename T, typename U>
concept HasLessThan = UT_REQUIRE_BINARY(a < b);
template<typename T, typename U>
concept HasLessThanEqual = UT_REQUIRE_BINARY(a <= b);


template<typename T, typename U>
concept HasGreaterThan = UT_REQUIRE_BINARY(a > b);
template<typename T, typename U>
concept HasGreaterThanEqual = UT_REQUIRE_BINARY(a >= b);


template<typename T, typename U>
concept HasEqual = UT_REQUIRE_BINARY(a == b);
template<typename T, typename U>
concept HasNotEqual = UT_REQUIRE_BINARY(a != b);


// clang-format off
template<typename T, typename U>
concept HasSpaceship = UT_REQUIRE_BINARY(a <=> b);
// clang-format on


template<typename T, typename U>
concept HasIndex = UT_REQUIRE_BINARY(a[b]);


template<typename T, typename... Args>
concept HasCall = requires(T t) {
    t(std::declval<Args>()...);
};

#undef UT_REQUIRE_UNARY
#undef UT_REQUIRE_BINARY

}  // namespace ut::detail


#endif  // CONCEPTS_HPP
