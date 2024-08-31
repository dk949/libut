#ifndef UT_OPTIONALOF_HPP
#define UT_OPTIONALOF_HPP
#include <optional>
#include <type_traits>

namespace ut {


template<typename T>
using OptionalOf = std::conditional_t<std::is_pointer_v<T>, T, std::optional<T>>;

template<typename T>
using ConstOptionalOf =
    std::conditional_t<std::is_pointer_v<T>, std::add_pointer_t<std::remove_pointer_t<T> const>, std::optional<T const>>;

template<typename T>
using PointerOf = std::conditional_t<std::is_pointer_v<T>, T, std::add_pointer_t<T>>;
template<typename T>
using ConstPointerOf = std::conditional_t<std::is_pointer_v<T>, ConstOptionalOf<T>, std::add_pointer_t<T const>>;

template<typename T>
[[nodiscard]]
OptionalOf<T> nullValueOf() noexcept {
    if constexpr (std::is_pointer_v<T>)
        return nullptr;
    else
        return std::nullopt;
}

template<typename T>
[[nodiscard]]
bool isNull(ConstOptionalOf<T> const &data) noexcept {
    if constexpr (std::is_pointer_v<T>)
        return data == nullptr;
    else
        return !data.has_value();
}

template<typename T>
[[nodiscard]]
T &getNonNullValue(OptionalOf<T> &data) noexcept {
    if constexpr (std::is_pointer_v<T>)
        return data;
    else
        return *data;
}

template<typename T>
PointerOf<T> operatorArrow(OptionalOf<T> &data) noexcept {
    if constexpr (std::is_pointer_v<T>)
        return data;
    else
        return data.operator->();
}

}  // namespace ut

#endif  // UT_OPTIONALOF_HPP
