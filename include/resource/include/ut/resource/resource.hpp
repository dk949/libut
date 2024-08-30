#ifndef UT_RESOURCE_HPP
#define UT_RESOURCE_HPP
#include "optionalof.hpp"

namespace ut {

template<typename T, typename D>
class Resource {
    OptionalOf<T> m_data = nullValueOf<T>();
    D m_destructor;

public:
    Resource(T &&data, D &&destructor) noexcept(std::is_nothrow_constructible_v<T, T &&>  //
                                                && std::is_nothrow_constructible_v<D, D &&>)
            : m_data(std::forward<T>(data))
            , m_destructor(std::forward<D>(destructor)) { }

    Resource(D &&destructor) noexcept(std::is_nothrow_constructible_v<D, D &&>)
            : m_destructor(std::forward<D>(destructor)) { }

    Resource(Resource const &) = delete;
    Resource &operator=(Resource const &) = delete;

    Resource(Resource &&) = default;
    Resource &operator=(Resource &&) = default;

    ~Resource() noexcept(noexcept(release())) {
        release();
    }

public:
    void acquire(T &&data) noexcept(noexcept(release()) && std::is_nothrow_move_assignable_v<T>) {
        release();
        m_data = std::move(data);
    }

    void release() noexcept(noexcept(m_destructor(getNonNullValue<T>(m_data)))
                            && std::is_nothrow_assignable_v<T, decltype(nullValueOf<T>())>) {
        if (!isNull<T>(m_data)) {
            m_destructor(getNonNullValue<T>(m_data));
            m_data = nullValueOf<T>();
        }
    }

    [[nodiscard]]
    OptionalOf<T> takeOwnership() noexcept(std::is_nothrow_move_assignable_v<T>  //
                                           && std::is_nothrow_assignable_v<T, decltype(nullValueOf<T>())>) {
        auto tmp = std::move(m_data);
        m_data = nullValueOf<T>();
        return tmp;
    }

    T &get() noexcept(false) {
        return tryGetNonNullValue<T>(m_data);
    }
};
}  // namespace ut

#endif  // UT_RESOURCE_HPP
