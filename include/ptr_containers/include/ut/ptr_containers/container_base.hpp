#ifndef CONTAINER_BASE_HPP
#define CONTAINER_BASE_HPP

#include "template_helpers.hpp"

#ifndef assert
#    include <cassert>
#endif
#include <cstddef>
#include <cstring>
#include <memory>
#include <stack>
#include <utility>

#define UT_CONTAINER_BASE_INJECT_DEPENDANT_NAMES(CName) \
public:                                                 \
    using typename CName::value_type;                   \
    using typename CName::reference;                    \
    using typename CName::const_reference;              \
    using typename CName::size_type;                    \
    using typename CName::difference_type;              \
    using typename CName::iterator;                     \
    using typename CName::const_iterator;               \
    using typename CName::reverse_iterator;             \
    using typename CName::const_reverse_iterator;       \
    using CName::end;                                   \
    using CName::begin;                                 \
    using CName::npos;                                  \
private:                                                \
    using CName::m_data;                                \
    using CName::m_size

namespace ut {

template<typename T,
    typename ValueType = T,
    typename StorageType = T *,  // Not one of the types used by the standard library
    typename Reference = T &,
    typename ConstReference = T const &,
    typename Iterator = T *,
    typename ConstIterator = T const *,
    typename ReverseIterator = std::reverse_iterator<Iterator>,
    typename ConstReverseIterator = std::reverse_iterator<ConstIterator>,
    typename SizeType = std::size_t,
    typename DifferenceType = std::ptrdiff_t>
class ContainerBase {
public:  ////////// types //////////
    using value_type = ValueType;
    using reference = Reference;
    using const_reference = ConstReference;
    using size_type = SizeType;
    using difference_type = DifferenceType;
    using iterator = Iterator;
    using const_iterator = ConstIterator;
    using reverse_iterator = ReverseIterator;
    using const_reverse_iterator = ConstReverseIterator;
    static inline size_type npos = ~(size_type(0));
protected:
    StorageType m_data;
    size_type m_size;

public:  ////////// element access //////////

    reference operator[](size_type idx) {
        assert(m_size > idx);
        return m_data[idx];
    }

    const_reference operator[](size_type idx) const {
        assert(m_size > idx);
        return m_data[idx];
    }

    reference front() {
        assert(m_size > 0);
        return m_data[0];
    }

    const_reference front() const {
        assert(m_size > 0);
        return m_data[0];
    }

    reference back() {
        assert(m_size > 0);
        return m_data[m_size - 1];
    }

    const_reference back() const {
        assert(m_size > 0);
        return m_data[m_size - 1];
    }

    StorageType data() {
        return m_data;
    }

    detail::AddTransitiveConstT<StorageType> data() const {
        return m_data;
    }
public:  ////////// iterators //////////

    iterator begin() {
        return m_data;
    }

    const_iterator begin() const {
        return m_data;
    }

    const_iterator cbegin() const {
        return m_data;
    }

    iterator end() {
        return m_data + m_size;
    }

    const_iterator end() const {
        return m_data + m_size;
    }

    const_iterator cend() const {
        return m_data + m_size;
    }

    reverse_iterator rbegin() {
        return reverse_iterator {end()};
    }

    const_reverse_iterator rbegin() const {
        return const_reverse_iterator {end()};
    }

    const_reverse_iterator crbegin() const {
        return const_reverse_iterator {cend()};
    }

    reverse_iterator rend() {
        return reverse_iterator {begin()};
    }

    const_reverse_iterator rend() const {
        return const_reverse_iterator {begin()};
    }

    const_reverse_iterator crend() const {
        return const_reverse_iterator {cbegin()};
    }

public:  ////////// capacity //////////
    bool empty() const {
        return m_size == 0;
    }

    size_type size() const {
        return m_size;
    }

    size_type max_size() const {
        // NOTE: may change later
        return ~(size_type(0));
    }
};

namespace detail {
    template<typename... Args>
    std::true_type derivedFromContainerBase(ContainerBase<Args...> *);
    template<typename...>
    std::false_type derivedFromContainerBase(...);
}

template<typename T>
struct IsDerivedFromContainerBase : decltype(detail::derivedFromContainerBase(std::declval<T *>())) { };

template<typename T>
inline constexpr bool IsDerivedFromContainerBaseV = IsDerivedFromContainerBase<T>::value;

template<typename T>
concept InheritsContainerBase = IsDerivedFromContainerBaseV<T>;

template<typename A, typename B>
struct IsComparableContainerBase : std::false_type { };

template<InheritsContainerBase T>
struct IsComparableContainerBase<T, T> : std::true_type { };

template<typename A, typename B>
inline constexpr bool IsComparableContainerBaseV = IsComparableContainerBase<A, B>::value;

template<typename A, typename B>
concept ComparableContainerBase = IsComparableContainerBaseV<A, B>;

template<typename A, typename B>
requires ComparableContainerBase<A, B>  //
    bool operator==(A const &a, B const &b) {

    using S = std::common_type_t<typename A::size_type, typename B::size_type>;

    if (detail::PtrCmp<A, B>()(&a, &b)) return true;

    if (a.size() != b.size()) return false;

    for (S i = 0; i < a.size(); ++i) {
        if (a.data()[i] == b.data()[i]) continue;
        if (*a.data()[i] != *b.data()[i]) return false;
    }

    return true;
}

}  // namespace ut

#endif  // CONTAINER_BASE_HPP
