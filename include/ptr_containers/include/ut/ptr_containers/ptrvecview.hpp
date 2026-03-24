#ifndef PTRVECVIEW_HPP
#define PTRVECVIEW_HPP

#if __cplusplus < 202'002L
#    error this file has to be compiled with at least C++20
#endif

#include "container_base.hpp"

#define UT_DETAIL_BASE                          \
    ContainerBase<T,                            \
        /*ValueType      = */ T,                \
        /*StorageType    = */ T const *const *, \
        /*Reference      = */ T const *,        \
        /*ConstReference = */ T const *,        \
        /*Iterator       = */ T const *const *, \
        /*ConstIterator  = */ T const *const *>

namespace ut {
template<typename T>
class PtrVecView : public UT_DETAIL_BASE {
    using Base = UT_DETAIL_BASE;
#undef UT_DETAIL_BASE

    UT_CONTAINER_BASE_INJECT_DEPENDANT_NAMES(Base);

public:
    PtrVecView() noexcept {
        m_data = nullptr;
        m_size = 0;
    }

    PtrVecView(const_iterator from, const_iterator to) noexcept(if_assert) {
        m_data = from;
        auto const dist = detail::distance(from, to);
        assert(dist > 0);
        m_size = size_type(dist);
    }

    PtrVecView(const_iterator from, size_type count) noexcept {
        m_data = from;
        m_size = count;
    }

    void swap(PtrVecView &other) noexcept {
        std::swap(m_data, other.m_data);
        std::swap(m_size, other.m_size);
    }
};

template<typename T>
void swap(ut::PtrVecView<T> &a, ut::PtrVecView<T> &b) noexcept {
    a.swap(b);
}

}  // namespace ut

namespace std {
template<typename T>
void swap(ut::PtrVecView<T> &a, ut::PtrVecView<T> &b) noexcept {
    a.swap(b);
}
}

#endif  // PTRVECVIEW_HPP
