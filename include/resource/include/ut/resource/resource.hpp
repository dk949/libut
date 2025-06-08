#ifndef UT_RESOURCE_HPP
#define UT_RESOURCE_HPP

#if __cplusplus < 202'002L
#    error this file has to be compiled with at least C++20
#endif

#include "optionalof.hpp"

#include <type_traits>

/* Generic resource holder with a destructor.
 *
 * Similar to std::uniqur_ptr, but:
 *  1. Works with any kind of resource, not just pointers
 *  2. Has better CTAD, including constructing a resource just from the constructor
 *
 * Usage:
 *
 * ```
 *     value_t getValueResource();
 *     void freeValueResource(value_t);
 *
 *     ptr_t getPtrResource();
 *     void freePtrResource(ptr_t);
 *     {
 *         ut::Resource value1 {getValueResource(), freeValueResource};
 *         ut::Resource value2 {freeValueResource};
 *         // `freeValueResource` will be called at the end of the scope for
 *         // value1 but not value2, because value2 has not been initialised.
 *     }
 *
 *     {
 *         ut::Resource value1 {getValueResource(), freeValueResource};
 *         ut::Resource value2 {freeValueResource};
 *
 *         value1.release(); // `freeValueResource` called here
 *
 *         value2.acquire(getValueResource());
 *
 *         // `freeValueResource` called for old resource here
 *         // and will be called again at the end of the scope for
 *         // the resource aquired here.
 *         value2.acquire(getValueResource());
 *
 *     }
 *
 *     {
 *         ut::Resource value {getValueResource(), freeValueResource};
 *         ut::Resource pointer {getPtrResource(), freePtrResource};
 *
 *         // `free*` functions will not be called
 *         // optional may be empty and pointer may be null!
 *         std::optional<value_t> value.takeOwnership();
 *         ptr_t pointer.takeOwnership();
 *     }
 *
 *     {
 *         ut::Resource value {getValueResource(), freeValueResource};
 *         ut::Resource pointer {getPtrResource(), freePtrResource};
 *
 *         value_t &v = value.get(); // UB if value is empty
 *         ptr_t p = ptr.get();      // pointer may be null
 *         auto x = value->x;        // UB if value is empty
 *         auto x = ptr->x;          // UB if value is empty
 *
 *         // Check if value is empty with `Resource::hasValue()`
 *     }
 *
 * ```
 */

namespace ut {

template<typename T, typename D>
class Resource {
private:

    mutable OptionalOf<T> m_data = nullValueOf<T>();
    D m_destructor;

private:
    static constexpr bool needs_internal_mutability = std::is_pointer_v<T> && !std::is_const_v<std::remove_pointer_t<T>>;

public:
    template<typename Tt, typename Dd>
    Resource(Tt &&data, Dd &&destructor) noexcept(std::is_nothrow_constructible_v<T, Tt &&>  //
                                                  && std::is_nothrow_constructible_v<D, Dd &&>)
            : m_data(std::forward<Tt>(data))
            , m_destructor(std::forward<Dd>(destructor)) { }

    Resource() noexcept(std::is_nothrow_default_constructible_v<D>)  //
        requires(std::is_default_constructible_v<D>)
            : m_destructor() { }

    template<typename Tt>
    explicit Resource(Tt &&data) noexcept(std::is_nothrow_constructible_v<T, Tt &&>) requires(  //
        std::is_same_v<std::remove_cvref_t<Tt>, std::remove_cvref_t<T>>                         //
            &&std::is_default_constructible_v<D>                                                //
        && !(std::is_pointer_v<D> && std::is_function_v<std::remove_pointer_t<D>>)              //
        )
            : m_data(std::forward<Tt>(data))
            , m_destructor {} { }

    template<typename Dd>
    explicit Resource(Dd &&destructor) noexcept(std::is_nothrow_constructible_v<D, Dd &&>)
        requires(std::is_constructible_v<D, Dd>)
            : m_destructor(std::forward<Dd>(destructor)) { }

    Resource(Resource const &) = delete;
    Resource &operator=(Resource const &) = delete;

    Resource(Resource &&other) noexcept(
        std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_constructible_v<D>)
            : m_data(std::move(other.m_data))
            , m_destructor(std::move(other.m_destructor)) {
        other.m_data = nullValueOf<T>();
    }

    Resource &operator=(Resource &&other) noexcept(
        std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_assignable_v<D>) {
        if (this == &other) return this;
        release();
        m_data = std::move(other.m_data);
        m_destructor = std::move(other.m_destructor);
        other.m_data = nullValueOf<T>();
    }

    ~Resource() noexcept(noexcept(release())) {
        release();
    }

public:
    [[nodiscard]]
    bool hasValue() const noexcept {
        return !isNull<T>(m_data);
    }

    [[nodiscard]] operator bool() const noexcept {
        return hasValue();
    }

    /**
     * Acquire  the resource.
     *
     * If resource was previously not empty, release it first
     */
    void acquire(T &&data) noexcept(noexcept(release()) && std::is_nothrow_move_assignable_v<T>) {
        release();
        m_data = std::move(data);
    }

    /**
     * Release the resource
     *
     * No-op  if resource was empty.
     *
     * NOTE: this is called automatically by the destructor
     */
    void release() noexcept(noexcept(m_destructor(getNonNullValue<T>(m_data)))
                            && std::is_nothrow_assignable_v<T, decltype(nullValueOf<T>())>) {
        if (!isNull<T>(m_data)) {
            m_destructor(getNonNullValue<T>(m_data));
            m_data = nullValueOf<T>();
        }
    }

    /**
     * Take ownership of the resource.
     *
     * The resource will *NOT* be automatically released by the destructor.
     *
     * Resource becomes empty.
     */
    [[nodiscard("If return value is discarded, resource may be leaked")]]
    OptionalOf<T> takeOwnership() noexcept(std::is_nothrow_move_assignable_v<T>  //
                                           && std::is_nothrow_assignable_v<T, decltype(nullValueOf<T>())>) {
        auto tmp = std::move(m_data);
        m_data = nullValueOf<T>();
        return tmp;
    }

    /**
     * Get a non-owning reference to the resource
     *
     * *UB if called on empty resource*, use `hasValue` to check first.
     */
    [[nodiscard]]
    T &get() noexcept {
        return getNonNullValue<T>(m_data);
    }

    /**
     * Get a non-owning reference to the resource
     *
     * *UB if called on empty resource*, use `hasValue` to check first.
     */
    [[nodiscard]]
    T const &get() const noexcept {
        return getNonNullValue<T>(m_data);
    }

    /**
     * Call `operator->` on the underlying value/pointer
     *
     * *UB if called on empty resource*, use `hasValue` to check first.
     */
    [[nodiscard]]
    PointerOf<T> operator->() noexcept {
        return operatorArrow<T>(m_data);
    }

    /**
     * Call `operator->` on the underlying value/pointer
     *
     * *UB if called on empty resource*, use `hasValue` to check first.
     */
    [[nodiscard]]
    std::conditional_t<needs_internal_mutability, PointerOf<T>, ConstPointerOf<T>> operator->() const noexcept {
        return operatorArrow<T>(m_data);
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

template<typename T>
auto malloced(T *t) {
    return Resource(t, [](auto *ptr) { free(ptr); });
}

}  // namespace ut

#endif  // UT_RESOURCE_HPP
