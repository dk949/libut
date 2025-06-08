#ifndef UT_STATIC_STRING_HPP
#define UT_STATIC_STRING_HPP

#if __cplusplus < 202002L
#error this file has to be compiled with at least C++20
#endif

/**Usage:
 * std >= c++20 (c++23)
 * // typedef using `char`
 * ut::StaticString my_str = "hello";     // Implicitly convertible from char const *
 * std::string_view my_sv = my_str;       // Implicitly convertible to string_view
 * ut::StaticString<10> my_str2{"str"sv}; // Explicitly convertible from string_view
 *                                        // NOTE: Overallocated space is filled with 0s
 *                                        //       Exception if not enough space.
 *
 *                                    // Behaves largely like std::string_view, except:
 *                                    //    * String is stored in the BasicStaticString
 *                                    //    * BasicStaticString is templated on type and size (and traits)
 *                                    //    * Contained string is mutable
 *
 * template<ut::StaticString Str>     //    * BasicStaticString can be passed as template parameters
 * void foo() {
 *     std::cout << Str << '\n';
 * }
 *
 * foo<"hello">(); // prints "hello"
 *
 *                                    //    * Two extension methods:
 *                                    //       - view() -> explicit cast to string_view
 *                                    //       - isNullTerminated() -> is the string null terninated
 *                                    //       - numNullBytes() -> how many null bytes are pas the
 *                                    //                           end of the string.
 *                                    //         NOTE: size() ignores traling null bytes (if any),
 *                                    //               use capacity() or max_size() to get the true
 *                                    //               capacity.
 *
 *
 * constexpr ut::StaticString foo = "hello world";  // Full constexpr support
 * constexpr auto res = foo.find(' ');
 */
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <string>
#include <string_view>
#include <utility>

#ifndef UT_DETAIL_THROW_OR_ABORT
#    ifdef __cpp_exceptions
#        include <stdexcept>
#        define UT_DETAIL_THROW_OR_ABORT(T, msg) throw T(msg)
#    else
#        include <cstdlib>
#        include <iostream>
#        define UT_DETAIL_THROW_OR_ABORT(T, msg) \
            do {                                 \
                std::cerr << (msg) << '\n';      \
                std::abort();                    \
            } while (0)
#    endif
#endif

namespace ut {


template<typename Char, std::size_t count, typename Traits = std::char_traits<Char>>
requires(count > 0) struct BasicStaticString {
    using traits_type = Traits;
    using value_type = Char;
    using pointer = Char *;
    using const_pointer = Char const *;
    using reference = Char &;
    using const_reference = Char const &;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    using view_type = std::basic_string_view<Char>;


public:  // Iterators

    [[nodiscard]]
    constexpr iterator begin() noexcept {
        return m_data;
    }

    [[nodiscard]]
    constexpr const_iterator begin() const noexcept {
        return m_data;
    }

    [[nodiscard]]
    constexpr const_iterator cbegin() const noexcept {
        return m_data;
    }

    [[nodiscard]]
    constexpr iterator end() noexcept {
        return m_data + size();
    }

    [[nodiscard]]
    constexpr const_iterator end() const noexcept {
        return m_data + size();
    }

    [[nodiscard]]
    constexpr const_iterator cend() const noexcept {
        return m_data + size();
    }

    [[nodiscard]]
    constexpr reverse_iterator rbegin() noexcept {
        return std::make_reverse_iterator(end());
    }

    [[nodiscard]]
    constexpr const_reverse_iterator rbegin() const noexcept {
        return std::make_reverse_iterator(end());
    }

    [[nodiscard]]
    constexpr const_reverse_iterator crbegin() const noexcept {
        return std::make_reverse_iterator(cend());
    }

    [[nodiscard]]
    constexpr reverse_iterator rend() noexcept {
        return std::make_reverse_iterator(begin());
    }

    [[nodiscard]]
    constexpr const_reverse_iterator rend() const noexcept {
        return std::make_reverse_iterator(begin());
    }

    [[nodiscard]]
    constexpr const_reverse_iterator crend() const noexcept {
        return std::make_reverse_iterator(cbegin());
    }


public:  // Element access
    [[nodiscard]]
    constexpr reference operator[](size_type idx) noexcept {
        return m_data[idx];
    }

    [[nodiscard]]
    constexpr const_reference operator[](size_type idx) const noexcept {
        return m_data[idx];
    }

    [[nodiscard]]
    constexpr reference at(size_type idx) {
        if (idx < size())
            return m_data[idx];
        else
            UT_DETAIL_THROW_OR_ABORT(std::out_of_range, "StaticString access out of range");
    }

    [[nodiscard]]
    constexpr const_reference at(size_type idx) const {
        if (idx < size())
            return m_data[idx];
        else
            UT_DETAIL_THROW_OR_ABORT(std::out_of_range, "StaticString access out of range");
    }

    [[nodiscard]]
    constexpr reference front() noexcept {
        return m_data[0];
    }

    [[nodiscard]]
    constexpr const_reference front() const noexcept {
        return m_data[0];
    }

    [[nodiscard]]
    constexpr reference back() noexcept {
        return m_data[size() - 1];
    }

    [[nodiscard]]
    constexpr const_reference back() const noexcept {
        return m_data[size() - 1];
    }

    [[nodiscard]]
    constexpr pointer data() noexcept {
        return m_data;
    }

    [[nodiscard]]
    constexpr const_pointer data() const noexcept {
        return m_data;
    }

    [[nodiscard]]
    constexpr operator view_type() const noexcept {
        return view_type {m_data, size()};
    }

public:  // Capacity
    [[nodiscard]]
    constexpr size_type size() const noexcept {
        return count - numNullBytes();
    }

    [[nodiscard]]
    constexpr size_type length() const noexcept {
        return size();
    }

    [[nodiscard]]
    constexpr size_type max_size() const noexcept {
        return count;
    }

    [[nodiscard]]
    constexpr size_type capacity() const noexcept {
        return max_size();
    }

    [[nodiscard]]
    constexpr bool empty() const noexcept {
        return size() == 0;
    }

public:  // Modifiers
    constexpr void swap(BasicStaticString &other) noexcept {
        std::swap(m_data, other.m_data);
    }


public:  // Operations
    constexpr size_type copy(Char *dest, size_type cnt, size_type pos = 0) const {
        return view().copy(dest, cnt, pos);
    }

    [[nodiscard]]
    constexpr view_type substr(size_type pos = 0, size_type cnt = npos) const {
        return view().substr(pos, cnt);
    }

    template<size_type Count = count, size_type Pos = 0>
    [[nodiscard]]
    constexpr BasicStaticString<Char, Pos + Count> substr() const noexcept requires(Pos + Count <= count) {
        return {m_data + Pos, Count};
    }

    template<size_type Count>
    [[nodiscard]]
    constexpr int compare(BasicStaticString<Char, Count> v) const noexcept {
        return view().compare(v.view());
    }

    template<size_type Count>
    [[nodiscard]]
    constexpr int compare(size_type pos1, size_type count1, BasicStaticString<Char, Count> v) const {
        return view().compare(pos1, count1, v.view());
    }

    template<size_type Count>
    [[nodiscard]]
    constexpr int compare(
        size_type pos1, size_type count1, BasicStaticString<Char, Count> v, size_type pos2, size_type count2) const {
        return view().compare(pos1, count1, v.view(), pos2, count2);
    }

    [[nodiscard]]
    constexpr int compare(view_type v) const noexcept {
        return view().compare(v);
    }

    [[nodiscard]]
    constexpr int compare(size_type pos1, size_type count1, view_type v) const {
        return view().compare(pos1, count1, v);
    }

    [[nodiscard]]
    constexpr int compare(size_type pos1, size_type count1, view_type v, size_type pos2, size_type count2) const {
        return view().compare(pos1, count1, v, pos2, count2);
    }

    [[nodiscard]]
    constexpr int compare(Char const *s) const {
        return view().compare(s);
    }

    [[nodiscard]]
    constexpr int compare(size_type pos1, size_type count1, Char const *s) const {
        return view().compare(pos1, count1, s);
    }

    [[nodiscard]]
    constexpr int compare(size_type pos1, size_type count1, Char const *s, size_type count2) const {
        return view().compare(pos1, count1, s, count2);
    }

    [[nodiscard]]
    constexpr bool starts_with(view_type sv) const noexcept {
        return view().starts_with(sv);
    }

    [[nodiscard]]
    constexpr bool starts_with(Char ch) const noexcept {
        return view().starts_with(ch);
    }

    [[nodiscard]]
    constexpr bool starts_with(Char const *s) const {
        return view().starts_with(s);
    }

    [[nodiscard]]
    constexpr bool ends_with(view_type sv) const noexcept {
        return view().ends_with(sv);
    }

    [[nodiscard]]
    constexpr bool ends_with(Char ch) const noexcept {
        return view().ends_with(ch);
    }

    [[nodiscard]]
    constexpr bool ends_with(Char const *s) const {
        return view().ends_with(s);
    }

#if __cplusplus >= 202'302
    [[nodiscard]]
    constexpr bool contains(view_type sv) const noexcept {
        return view().contains(sv);
    }

    [[nodiscard]]
    constexpr bool contains(Char c) const noexcept {
        return view().contains(c);
    }

    [[nodiscard]]
    constexpr bool contains(Char const *s) const {
        return view().contains(s);
    }
#endif

    [[nodiscard]]
    constexpr size_type find(view_type v, size_type pos = 0) const noexcept {
        return view().find(v, pos);
    }

    [[nodiscard]]
    constexpr size_type find(Char ch, size_type pos = 0) const noexcept {
        return view().find(ch, pos);
    }

    [[nodiscard]]
    constexpr size_type find(Char const *s, size_type pos, size_type cnt) const {
        return view().find(s, pos, cnt);
    }

    [[nodiscard]]
    constexpr size_type find(Char const *s, size_type pos = 0) const {
        return view().find(s, pos);
    }

    [[nodiscard]]
    constexpr size_type rfind(view_type v, size_type pos = 0) const noexcept {
        return view().rfind(v, pos);
    }

    [[nodiscard]]
    constexpr size_type rfind(Char ch, size_type pos = 0) const noexcept {
        return view().rfind(ch, pos);
    }

    [[nodiscard]]
    constexpr size_type rfind(Char const *s, size_type pos, size_type cnt) const {
        return view().rfind(s, pos, cnt);
    }

    [[nodiscard]]
    constexpr size_type rfind(Char const *s, size_type pos = 0) const {
        return view().rfind(s, pos);
    }

    [[nodiscard]]
    constexpr size_type find_first_of(view_type v, size_type pos = 0) const noexcept {
        return view().find_first_of(v, pos);
    }

    [[nodiscard]]
    constexpr size_type find_first_of(Char ch, size_type pos = 0) const noexcept {
        return view().find_first_of(ch, pos);
    }

    [[nodiscard]]
    constexpr size_type find_first_of(Char const *s, size_type pos, size_type cnt) const {
        return view().find_first_of(s, pos, cnt);
    }

    [[nodiscard]]
    constexpr size_type find_first_of(Char const *s, size_type pos = 0) const {
        return view().find_first_of(s, pos);
    }

    [[nodiscard]]
    constexpr size_type find_last_of(view_type v, size_type pos = 0) const noexcept {
        return view().find_last_of(v, pos);
    }

    [[nodiscard]]
    constexpr size_type find_last_of(Char ch, size_type pos = 0) const noexcept {
        return view().find_last_of(ch, pos);
    }

    [[nodiscard]]
    constexpr size_type find_last_of(Char const *s, size_type pos, size_type cnt) const {
        return view().find_last_of(s, pos, cnt);
    }

    [[nodiscard]]
    constexpr size_type find_last_of(Char const *s, size_type pos = 0) const {
        return view().find_last_of(s, pos);
    }

    [[nodiscard]]
    constexpr size_type find_first_not_of(view_type v, size_type pos = 0) const noexcept {
        return view().find_first_not_of(v, pos);
    }

    [[nodiscard]]
    constexpr size_type find_first_not_of(Char ch, size_type pos = 0) const noexcept {
        return view().find_first_not_of(ch, pos);
    }

    [[nodiscard]]
    constexpr size_type find_first_not_of(Char const *s, size_type pos, size_type cnt) const {
        return view().find_first_not_of(s, pos, cnt);
    }

    [[nodiscard]]
    constexpr size_type find_first_not_of(Char const *s, size_type pos = 0) const {
        return view().find_first_not_of(s, pos);
    }

    [[nodiscard]]
    constexpr size_type find_last_not_of(view_type v, size_type pos = 0) const noexcept {
        return view().find_last_not_of(v, pos);
    }

    [[nodiscard]]
    constexpr size_type find_last_not_of(Char ch, size_type pos = 0) const noexcept {
        return view().find_last_not_of(ch, pos);
    }

    [[nodiscard]]
    constexpr size_type find_last_not_of(Char const *s, size_type pos, size_type cnt) const {
        return view().find_last_not_of(s, pos, cnt);
    }

    [[nodiscard]]
    constexpr size_type find_last_not_of(Char const *s, size_type pos = 0) const {
        return view().find_last_not_of(s, pos);
    }


public:  // comparisons

    template<std::size_t S>
    [[nodiscard]]
    constexpr bool operator==(BasicStaticString<Char, S> const &other) const noexcept {
        return view() == other.view();
    }

    [[nodiscard]]
    constexpr bool operator==(std::basic_string_view<Char> other) const noexcept {
        return view() == other;
    }

    // clang-format off
    template<std::size_t S>
    [[nodiscard]]
    constexpr auto operator<=>(BasicStaticString<Char, S> const &other) const noexcept {
        return view() <=> other.view();
    }

    [[nodiscard]]
    constexpr auto operator<=>(std::basic_string_view<Char> other) const noexcept {
        return view() <=> other;
    }

    // clang-format on


public:  // extensions
    [[nodiscard]]
    constexpr bool isNullTerminated() const noexcept {
        return m_data[count - 1] == 0;
    }

    constexpr std::size_t numNullBytes() const noexcept {
        auto const v = view_type {m_data, count};
        auto pos = v.find_last_not_of(Char(0));
        if (pos == v.npos)
            return v.size();
        else
            return v.size() - 1 - pos;
    }

    [[nodiscard]]
    constexpr view_type view() const noexcept {
        return *this;
    }

public:
    BasicStaticString() = default;

    constexpr BasicStaticString(const_pointer p, size_type s) noexcept {
        traits_type::copy(m_data, p, s);
    }

    explicit constexpr BasicStaticString(view_type sv) {
        if (sv.size() > count)
            UT_DETAIL_THROW_OR_ABORT(std::out_of_range, "Cannot construct BasicStaticString, string view too long");
        traits_type::copy(m_data, sv.data(), sv.size());
        for (auto it = m_data + sv.size(); it < m_data + count; ++it) {
            *it = 0;
        }
    }

    template<typename T, std::size_t S>
    constexpr BasicStaticString(T (&s)[S]) noexcept {
        traits_type::copy(m_data, s, S);
    }


public:  // Data members must be public
    Char m_data[count];
    static constexpr auto npos = count;
private:
};

template<typename T, std::size_t S>
BasicStaticString(T const (&)[S]) -> BasicStaticString<T, S>;


template<std::size_t S>
using StaticString = BasicStaticString<char, S>;
template<std::size_t S>
using WStaticString = BasicStaticString<wchar_t, S>;
template<std::size_t S>
using U8StaticString = BasicStaticString<char8_t, S>;
template<std::size_t S>
using U16StaticString = BasicStaticString<char16_t, S>;
template<std::size_t S>
using U32StaticString = BasicStaticString<char32_t, S>;

template<typename Char, std::size_t S>
std::basic_ostream<Char> &operator<<(std::basic_ostream<Char> &os, BasicStaticString<Char, S> const &s) {
    os << s.data();
    return os;
}

template<typename T, std::size_t count>
constexpr void swap(BasicStaticString<T, count> &a, BasicStaticString<T, count> &b) noexcept {
    a.swap(b);
}

}  // namespace ut

#endif  // UT_STATIC_STRING_HPP
