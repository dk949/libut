#ifndef OWNPTRVEC_HPP
#define OWNPTRVEC_HPP

#include "container_base.hpp"
#include "ptrvecview.hpp"
#include "template_helpers.hpp"

#ifndef assert
#    include <cassert>
#endif
#include <cstddef>
#include <cstring>
#include <memory>
#include <stack>
#include <utility>

namespace ut {

#define BASE                             \
    ContainerBase<T,                     \
        /*ValueType      = */ T,         \
        /*StorageType    = */ T **,      \
        /*Reference      = */ T *,       \
        /*ConstReference = */ T const *, \
        /*Iterator       = */ T **,      \
        /*ConstIterator  = */ T *const *>

template<typename T>
class OwnPtrVec : public BASE {
    using Base = BASE;

    UT_CONTAINER_BASE_INJECT_DEPENDANT_NAMES(Base);

private:

    size_type m_cap = 0;

    static constexpr auto multiplier = 1.5;

public:  ////////// constructors //////////

    static OwnPtrVec fromReserve(size_type res) {
        return OwnPtrVec(new T *[res], 0, res);
    }

    template<typename... Args>
    static OwnPtrVec make(Args &&...args) {
        auto vec = OwnPtrVec(new T *[sizeof...(Args)], 0, sizeof...(Args));
        vec.makeImpl(std::forward<Args>(args)...);
        return vec;
    }

    OwnPtrVec() {
        m_data = nullptr;
        m_size = 0;
        m_cap = 0;
    }

    OwnPtrVec(OwnPtrVec const &) = delete;
    OwnPtrVec &operator=(OwnPtrVec const &) = delete;

    OwnPtrVec(OwnPtrVec &&other) {
        m_size = 0;
        m_cap = 0;
        m_data = nullptr;
        *this = std::move(other);
    }

    OwnPtrVec &operator=(OwnPtrVec &&other) {
        deleteData();
        std::swap(m_data, other.m_data);
        std::swap(m_size, other.m_size);
        std::swap(m_cap, other.m_cap);
        return *this;
    }

    ~OwnPtrVec() {
        static_assert(sizeof(T) >= 0, "Cannot have incomplete type in the constructor");
        deleteData();
    }

    /** Create a (non-owning) view of the vector.
     *
     * `from`: starting index (inclusive)
     * `to`: ending index (not inclusive)
     *
     * NOTE: Use the PtrVecView constructor for an API that takes a starting index and a size
     */
    PtrVecView<T> view(size_type from = 0, size_type to = npos) {
        assert(from <= to);
        assert(from <= m_size);
        assert((to == npos || to <= m_size));
        return PtrVecView<T>(begin() + from, (to == npos ? m_size : to) - from);
    }

    /** Create a (non-owning) view of the vector.
     *
     * `from`: starting iterator (inclusive)
     * `to`: ending iterator (not inclusive)
     */
    static PtrVecView<T> view(const_iterator from, const_iterator to) {
        assert(from <= to);
        return PtrVecView<T>(from, to);
    }

private:
    OwnPtrVec(T **d, size_type s, size_type c) {
        m_data = d;
        m_size = s;
        m_cap = c;
    }

public:  ////////// element access //////////
    // void assign();

    // T *at();


    /** Caller owns returned memory.
     *
     *  Each element has to be deleted with `delete`, then the buffer needs to
     *  be deleted with `delete[]`.
     *
     *  The returned buffer can hold up to `capacity()` elements, but only the
     *  first `size()` are allocated (and hence have to be deleted).
     *
     *  Use of this function is strongly discouraged
     *
     *  NOTE: Call `size()` and `calacity()` *before* calling `release()` as they will
     *  be reset after the call.
     */
    [[nodiscard("returns owning pointer")]]
    T **release() {
        auto **ptr = m_data;
        m_data = nullptr;
        m_size = 0;
        m_cap = 0;
        return ptr;
    }

    /// Caller owns returned memory.
    [[nodiscard("object will be deleted at the end of the function call if not saved to temporary")]]
    std::unique_ptr<T> release_back() {
        return std::unique_ptr<T> {m_data[--m_size]};
    }

public:  ////////// capacity //////////
    void reserve(size_type new_capacity) {
        if (new_capacity <= m_cap) return;
        changeCapacity(new_capacity);
    }

    size_type capacity() const {
        return m_cap;
    }

    void shrink_to_fit() {
        changeCapacity(m_size);
    }

public:  ////////// modifiers //////////
    void clear() {
        for (size_type i = 0; i < m_size; ++i)
            delete m_data[i];
        m_size = 0;
    }

    iterator insert(const_iterator pos, std::unique_ptr<T> t) {
        return insertImpl(pos, t.release());
    }

    template<detail::DerivedOrEqualTo<T> U>
    iterator insert(const_iterator pos, U &&t) {
        using Ctor = std::remove_cvref_t<U>;
        return insertImpl(pos, new Ctor(std::forward<U>(t)));
    }

    iterator erase(iterator pos) {
        return erase(const_iterator(pos));
    }

    iterator erase(const_iterator pos) {
        return erase(pos, pos + 1);
    }

    iterator erase(const_iterator first, const_iterator last) {
        if (first == last) return m_data + detail::distance(m_data, last);
        assert(first != end());
        assert(m_size > 0);
        auto const range_size = detail::distance(first, last);
        assert(range_size > 0);
        assert(size_type(range_size) <= m_size);
        auto const start_idx = detail::distance(m_data, first);
        for (auto i = start_idx; i < start_idx + range_size; ++i)
            delete m_data[i];

        std::memmove(m_data + start_idx,
            m_data + start_idx + range_size,
            (m_size - size_type(start_idx) - size_type(range_size)) * sizeof(T *));
        m_size -= size_type(range_size);
        return m_data + start_idx;
    }

    template<detail::DerivedOrEqualTo<T> U = T, typename... Args>
    iterator emplace(const_iterator pos, Args &&...t) {
        using Ctor = std::remove_cvref_t<U>;
        return insertImpl(pos, new Ctor(std::forward<Args>(t)...));
    }

    void push_back(std::unique_ptr<T> t) {
        ensureExtraCapacity(1);
        m_data[m_size++] = t.release();
    }

    template<detail::DerivedOrEqualTo<T> U>
    void push_back(U &&t) {
        using Ctor = std::remove_cvref_t<U>;
        ensureExtraCapacity(1);
        m_data[m_size++] = new Ctor(std::forward<U>(t));
    }

    /// Construct new item in-place
    template<detail::DerivedOrEqualTo<T> U = T, typename... Args>
    reference emplace_back(Args &&...args) {
        using Ctor = std::remove_cvref_t<U>;
        ensureExtraCapacity(1);
        return m_data[m_size++] = new Ctor(std::forward<Args>(args)...);
    }

    void pop_back() {
        delete m_data[--m_size];
    }

    void swap(OwnPtrVec &other) noexcept {
        std::swap(m_data, other.m_data);
        std::swap(m_size, other.m_size);
        std::swap(m_cap, other.m_cap);
    }

private:
    void deleteData() {
        if (!m_data) {
            assert(!m_cap);
            assert(!m_size);
            return;
        }
        clear();
        delete[] m_data;
        m_data = nullptr;
        m_size = 0;
        m_cap = 0;
    }

    void changeCapacity(size_type to) {
        if (to == m_cap) return;
        auto *tmp = new T *[to]();
        assert(to >= m_size);
        if (m_size) {
            assert(m_data);
            std::memcpy(tmp, m_data, sizeof(T *) * m_size);
        }
        m_cap = to;
        std::swap(m_data, tmp);
        delete[] tmp;
    }

    void ensureExtraCapacity(size_type elems) {
        auto new_cap = calcCapacity(m_cap, m_size + elems);
        if (m_cap > new_cap) return;
        changeCapacity(new_cap);
    }

    iterator insertImpl(const_iterator pos, T *t) {
        if (pos == end()) {
            push_back(std::unique_ptr<T> {t});
            return end() - 1;
        }
        auto const idx = detail::distance(m_data, pos);
        assert(idx >= 0);
        assert(size_type(idx) <= m_size);
        if (m_size + 1 >= m_cap) {
            m_cap = calcCapacity(m_cap, m_size + 1);
            auto *tmp = new T *[m_cap];
            std::memcpy(tmp, m_data, size_type(idx) * sizeof(T *));
            std::memcpy(tmp + idx + 1, m_data + idx, (m_size - size_type(idx)) * sizeof(T *));
            std::swap(m_data, tmp);
            delete[] tmp;
        } else
            std::memmove(m_data + idx + 1, m_data + idx, (m_size - size_type(idx)) * sizeof(T *));

        ++m_size;
        m_data[idx] = t;
        return m_data + idx;
    }

    template<detail::UniquePtr First, typename... Args>
    void makeImpl(First &&first, Args &&...args)  //
        requires detail::DerivedOrEqualTo<typename First::element_type, T> {

        m_data[m_size++] = first.release();
        if constexpr (sizeof...(Args)) makeImpl(std::forward<Args>(args)...);
    }

    template<detail::NotUniquePtr First, typename... Args>
    void makeImpl(First &&first, Args &&...args)  //
        requires detail::DerivedOrEqualTo<First, T> {

        using Ctor = std::remove_cvref_t<First>;
        m_data[m_size++] = new Ctor(std::forward<First>(first));
        if constexpr (sizeof...(Args)) makeImpl(std::forward<Args>(args)...);
    }

    [[nodiscard]]
    static constexpr size_type calcCapacity(size_type old, size_type new_) {
        if (old >= new_) return new_;
        old = old < 2 ? 2 : old;
        for (; old < new_; old = size_type(double(old) * multiplier)) { }
        return old;
    }

#undef BASE
};

template<typename T>
using OwnPtrStack = std::stack<T, OwnPtrVec<T>>;

template<typename T>
struct IsComparableContainerBase<OwnPtrVec<T>, PtrVecView<T>> : std::true_type { };

template<typename T>
struct IsComparableContainerBase<PtrVecView<T>, OwnPtrVec<T>> : std::true_type { };

template<typename T>
void swap(ut::OwnPtrVec<T> &a, ut::OwnPtrVec<T> &b) {
    a.swap(b);
}

}  // namespace ut

namespace std {
template<typename T>
void swap(ut::OwnPtrVec<T> &a, ut::OwnPtrVec<T> &b) {
    a.swap(b);
}
}

#endif  // OWNPTRVEC_HPP
