#ifndef VALUEPTR_HPP
#define VALUEPTR_HPP
#include "template_helpers.hpp"

#include <memory>
#include <utility>

namespace ut {

template<typename T>
class ValuePtr {
    template<typename U>
    friend class ValuePtr;

private:
    T *m_obj = nullptr;

public:  ////////// constructors //////////
    ValuePtr()
            : m_obj(new T {}) { }

    template<detail::DerivedOrEqualTo<T> U>
    ValuePtr(U &&obj) {
        m_obj = new std::remove_cvref_t<U>(std::forward<U>(obj));
    }

    template<detail::DerivedOrEqualTo<T> U>
    explicit ValuePtr(std::unique_ptr<U> obj) {
        m_obj = obj.release();
    }

    ValuePtr(ValuePtr const &other) {
        m_obj = new T(*other.m_obj);
    }

    template<detail::DerivedOrEqualTo<T> U>
    ValuePtr(ValuePtr<U> const &other) {
        m_obj = new std::remove_cvref_t<U>(*other.m_obj);
    }

    ValuePtr &operator=(ValuePtr const &other) {
        delete m_obj;
        m_obj = new T(*other.m_obj);
        return *this;
    }

    template<detail::DerivedOrEqualTo<T> U>
    ValuePtr &operator=(ValuePtr<U> const &other) {
        delete m_obj;
        m_obj = new std::remove_cvref_t<U>(*other.m_obj);
        return *this;
    }

    template<detail::DerivedOrEqualTo<T> U>
    ValuePtr &operator=(U &&other) {
        delete m_obj;
        m_obj = new std::remove_cvref_t<U>(std::forward<U>(other));
        return *this;
    }

    template<detail::DerivedOrEqualTo<T> U>
    ValuePtr(ValuePtr<U> &&other) {
        *this = std::move(other);
    }

    template<detail::DerivedOrEqualTo<T> U>
    ValuePtr &operator=(ValuePtr<U> &&other) {
        delete m_obj;
        m_obj = other.m_obj;
        other.m_obj = nullptr;
        return *this;
    }

    ~ValuePtr() {
        delete m_obj;
        m_obj = nullptr;
    }

public:  ////////// functions //////////

    /** Get the underlying pointer.
     *
     * ValuePtr still owns the memeory - pointer will become invalid when this
     * object goes out of scope.
     *
     * \note This API should rarely be used. To obtain a reference to the
     *       contained object use `operator*`
     */
    T *get() {
        return m_obj;
    }

    T const *get() const {
        return m_obj;
    }

    /** Get ownership of the underlying pointer
     */
    std::unique_ptr<T> release() {
        auto *tmp = m_obj;
        m_obj = nullptr;
        return std::unique_ptr<T> {tmp};
    }

    void swap(ValuePtr &other) {
        std::swap(m_obj, other.m_obj);
    }

public:  ////////// operators //////////
    T *operator->() {
        return m_obj;
    }

    T const *operator->() const {
        return m_obj;
    }

    T &operator*() {
        return *m_obj;
    }

    T const &operator*() const {
        return *m_obj;
    }

private:
    template<typename U>
    using OpRetT = std::conditional_t<detail::DerivedOrEqualTo<U, T>, ValuePtr, U>;
#define UT_OP_RET(op) OpRetT<decltype(*m_obj op std::declval<U>())>

public:

    template<typename U>
    bool operator==(U const &u) const requires(detail::HasEqual<T, U>) {
        return *m_obj == u;
    }

    template<typename U>
    bool operator==(ValuePtr<U> const &u) const requires(detail::HasEqual<T, U>) {
        return *m_obj == *u.m_obj;
    }

    template<typename... Args>
    OpRetT<std::invoke_result_t<T, Args...>> operator()(Args &&...args) requires(detail::HasCall<T, Args...>) {
        return OpRetT<std::invoke_result_t<T, Args...>> {(*m_obj)(std::forward<Args>(args)...)};
    }

    template<typename... Args>
    OpRetT<std::invoke_result_t<T, Args...>> operator()(Args &&...args) const requires(detail::HasCall<T, Args...>) {
        return OpRetT<std::invoke_result_t<T, Args...>> {(*m_obj)(std::forward<Args>(args)...)};
    }

#undef UT_OP_RET
};

template<typename T>
ValuePtr(T t) -> ValuePtr<T>;

/// This overload is called in ADL use of swap
template<typename T>
void swap(ut::ValuePtr<T> &a, ut::ValuePtr<T> &b) {
    a.swap(b);
}
}  // namespace ut

namespace std {
/// This overload is called when swap is invoked as `std::swap`
template<typename T>
void swap(ut::ValuePtr<T> &a, ut::ValuePtr<T> &b) {
    a.swap(b);
}
}

#endif  // VALUEPTR_HPP
