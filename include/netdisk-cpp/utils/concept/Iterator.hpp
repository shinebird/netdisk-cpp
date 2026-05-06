#pragma once

// This header is C++ 20 version of boost/concept_check.hpp

#include <concepts>
#include <iterator>
#include <type_traits>

#include <netdisk-cpp/utils/concept/Common.hpp>

namespace netdisk::utils::reflect
{
    template <typename T>
    concept InputIterator =
        Assignable<T> && EqualityComparable<T> &&
        requires(T t) {
            typename std::iterator_traits<T>::value_type;
            typename std::iterator_traits<T>::difference_type;
            typename std::iterator_traits<T>::reference;
            typename std::iterator_traits<T>::pointer;
            typename std::iterator_traits<T>::iterator_category;
            { T(t) };
            { (void)*t };
            { ++t };
            { t++ };
        } && std::is_signed_v<typename std::iterator_traits<T>::difference_type> &&
        std::is_convertible_v<typename std::iterator_traits<T>::iterator_category,
                              std::input_iterator_tag>;

    template <typename T, typename ValueT>
    concept OutputIterator = Assignable<T> && requires(T t, ValueT value) {
        { ++t };
        { t++ };
        { *t++ = value };
    };

    template <typename T>
    concept ForwardIterator =
        InputIterator<T> &&
        requires(T t) {
            typename std::iterator_traits<T>::reference;
            { *t } -> std::convertible_to<typename std::iterator_traits<T>::reference>;
        } &&
        std::is_convertible_v<typename std::iterator_traits<T>::iterator_category,
                              std::forward_iterator_tag>;

    template <typename T>
    concept MutableForwardIterator = ForwardIterator<T> && requires(T t1, T t2) {
        { *t1++ = *t2 };
    };

    template <typename T>
    concept BidirectionalIterator =
        ForwardIterator<T> &&
        requires(T t) {
            { --t };
            { t-- };
        } &&
        std::is_convertible_v<typename std::iterator_traits<T>::iterator_category,
                              std::bidirectional_iterator_tag>;

    template <typename T>
    concept MutableBidirectionalIterator =
        BidirectionalIterator<T> && MutableForwardIterator<T> && requires(T t1, T t2) {
            { *t1-- = *t2 };
        };

    template <typename T>
    concept RandomAccessIterator =
        BidirectionalIterator<T> && Comparable<T> &&
        requires(T t1, T t2, typename std::iterator_traits<T>::difference_type n) {
            { t1 += n };
            { t1 = t1 + n };
            { t1 = n + t1 };
            { t1 -= n };
            { t1 = t1 - n };
            { n = t1 - t2 };
            { (void)t1[n] };
        } &&
        std::is_convertible_v<typename std::iterator_traits<T>::iterator_category,
                              std::random_access_iterator_tag>;

    template <typename T>
    concept MutableRandomAccessIterator =
        RandomAccessIterator<T> && MutableBidirectionalIterator<T> &&
        requires(T t, typename std::iterator_traits<T>::difference_type n) {
            { t[n] = *t };
        };
} // namespace solar
