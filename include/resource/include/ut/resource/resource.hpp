#ifndef UT_RESOURCE_HPP
#define UT_RESOURCE_HPP
#include "optionalof.hpp"

namespace ut {

template<typename T, typename D>
class Resource {
    OptionalOf<T> m_data = nullValueOf<T>();
    D m_destructor;

public:
    template<typename Tt, typename Dd>
    Resource(Tt &&data, Dd &&destructor) noexcept(std::is_nothrow_constructible_v<T, Tt &&>  //
                                                  && std::is_nothrow_constructible_v<D, Dd &&>)
            : m_data(std::forward<Tt>(data))
            , m_destructor(std::forward<Dd>(destructor)) { }

    template<typename Dd>
    Resource(Dd &&destructor) noexcept(std::is_nothrow_constructible_v<D, Dd &&>)
            : m_destructor(std::forward<Dd>(destructor)) { }

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

namespace detail {
    template<typename T>
    concept Object = std::is_object_v<T>;

    template<typename F, typename Ret, typename A, typename... Rest>
    A deduceMemFn(Ret (F::*)(A, Rest...));

    template<typename F, typename Ret, typename A, typename... Rest>
    A deduceMemFn(Ret (F::*)(A, Rest...) const);

    template<typename F>
    struct GetArg;

    template<typename R, typename A>
    struct GetArg<R(A)> {
        using Arg = A;
    };

    template<Object O>
    struct GetArg<O> {
        using Arg = decltype(deduceMemFn(&O::operator()));
    };

}  // namespace detail

template<typename Tt, typename Dd>
Resource(Tt &&data, Dd &&destructor) -> Resource<std::decay_t<Tt>, std::decay_t<Dd>>;


template<typename Dd>
Resource(Dd &&destructor)
    -> Resource<std::decay_t<typename ut::detail::GetArg<std::remove_pointer_t<std::decay_t<Dd>>>::Arg>, std::decay_t<Dd>>;

}  // namespace ut

#endif  // UT_RESOURCE_HPP
