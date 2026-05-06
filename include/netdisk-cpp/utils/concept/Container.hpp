#pragma once

#include <concepts>

#include <netdisk-cpp/utils/concept/Common.hpp>
#include <netdisk-cpp/utils/concept/Iterator.hpp>

namespace netdisk::utils::reflect
{
    template <typename C>
    concept Container = requires(C container) {
        typename C::value_type;
        typename C::difference_type;
        typename C::size_type;
        typename C::const_reference;
        typename C::const_pointer;
        typename C::const_iterator;
        { container.begin() } -> std::convertible_to<typename C::const_iterator>;
        { container.end() } -> std::convertible_to<typename C::const_iterator>;
        { container.size() } -> std::same_as<typename C::size_type>;
        { container.max_size() } -> std::same_as<typename C::size_type>;
        { container.empty() } -> std::same_as<bool>;
    } && InputIterator<typename C::const_iterator>;

    template <typename C>
    concept MutableContainer = Container<C> && requires(C container, C container1) {
        typename C::reference;
        typename C::iterator;
        typename C::pointer;
        { container.begin() } -> std::convertible_to<typename C::iterator>;
        { container.end() } -> std::convertible_to<typename C::iterator>;
        { container.swap(container1) } -> std::same_as<void>;
    } && Assignable<typename C::value_type> && InputIterator<typename C::iterator>;
    ;

    template <typename C>
    concept ForwardContainer = Container<C> && ForwardIterator<typename C::const_iterator>;

    template <typename C>
    concept MutableForwardContainer =
        MutableContainer<C> && MutableForwardIterator<typename C::iterator>;

    template <typename C>
    concept ReversibleContainer = ForwardContainer<C> &&
                                  requires(C container) {
                                      typename C::const_reverse_iterator;
                                      {
                                          container.rbegin()
                                      } -> std::convertible_to<typename C::const_reverse_iterator>;
                                      {
                                          container.rend()
                                      } -> std::convertible_to<typename C::const_reverse_iterator>;
                                  } && BidirectionalIterator<typename C::const_iterator> &&
                                  BidirectionalIterator<typename C::const_reverse_iterator>;

    template <typename C>
    concept MutableReversibleContainer = MutableForwardContainer<C> && ReversibleContainer<C> &&
                                         requires(C container) {
                                             typename C::reverse_iterator;
                                             {
                                                 container.rbegin()
                                             } -> std::convertible_to<typename C::reverse_iterator>;
                                             {
                                                 container.rend()
                                             } -> std::convertible_to<typename C::reverse_iterator>;
                                         } && MutableBidirectionalIterator<typename C::iterator> &&
                                         MutableBidirectionalIterator<typename C::reverse_iterator>;

    template <typename C>
    concept RandomAccessContainer =
        ReversibleContainer<C> && requires(C container, typename C::size_type n) {
            typename C::size_type;
            typename C::const_reference;
            { container[n] } -> std::convertible_to<typename C::const_reference>;
        } && RandomAccessIterator<typename C::const_iterator>;

    template <typename C>
    concept MutableRandomAccessContainer =
        MutableReversibleContainer<C> && RandomAccessContainer<C> &&
        requires(C container, typename C::size_type n) {
            typename C::reference;
            { container[n] } -> std::convertible_to<typename C::reference>;
        } && MutableRandomAccessIterator<typename C::iterator> &&
        MutableRandomAccessIterator<typename C::reverse_iterator>;

    // C++ 20 span specific
    // span在C++20中缺少const_iterator和const_reverse_iterator

    template <typename C>
    concept ContainerSpan = requires(C container) {
        typename C::value_type;
        typename C::difference_type;
        typename C::size_type;
        typename C::const_reference;
        typename C::const_pointer;
        // typename C::const_iterator;
        // { container.begin() } -> std::convertible_to<typename C::const_iterator>;
        // { container.end() } -> std::convertible_to<typename C::const_iterator>;
        { container.size() } -> std::same_as<typename C::size_type>;
        // { container.max_size() } -> std::same_as<typename C::size_type>;
        { container.empty() } -> std::same_as<bool>;
    } /*&& InputIterator<typename C::const_iterator>*/;

    template <typename C>
    concept MutableContainerSpan = ContainerSpan<C> && requires(C container, C container1) {
        typename C::reference;
        typename C::iterator;
        typename C::pointer;
        { container.begin() } -> std::convertible_to<typename C::iterator>;
        { container.end() } -> std::convertible_to<typename C::iterator>;
        // { container.swap(container1) } -> std::same_as<void>;
    } && Assignable<typename C::value_type> && InputIterator<typename C::iterator>;
    ;

    template <typename C>
    concept ForwardContainerSpan =
        ContainerSpan<C> /*&& ForwardIterator<typename C::const_iterator>*/;

    template <typename C>
    concept MutableForwardContainerSpan =
        MutableContainerSpan<C> && MutableForwardIterator<typename C::iterator>;

    template <typename C>
    concept ReversibleContainerSpan = ForwardContainerSpan<C> /*&&
                                  requires(C container) {
                                      typename C::const_reverse_iterator;
                                      {
                                          container.rbegin()
                                      } -> std::convertible_to<typename C::const_reverse_iterator>;
                                      {
                                          container.rend()
                                      } -> std::convertible_to<typename C::const_reverse_iterator>;
                                  } && BidirectionalIterator<typename C::const_iterator> &&
                                  BidirectionalIterator<typename C::const_reverse_iterator>*/
        ;

    template <typename C>
    concept MutableReversibleContainerSpan =
        MutableForwardContainerSpan<C> && ReversibleContainerSpan<C> &&
        requires(C container) {
            typename C::reverse_iterator;
            { container.rbegin() } -> std::convertible_to<typename C::reverse_iterator>;
            { container.rend() } -> std::convertible_to<typename C::reverse_iterator>;
        } && MutableBidirectionalIterator<typename C::iterator> &&
        MutableBidirectionalIterator<typename C::reverse_iterator>;

    template <typename C>
    concept RandomAccessContainerSpan =
        ReversibleContainerSpan<C> && requires(C container, typename C::size_type n) {
            typename C::size_type;
            typename C::const_reference;
            { container[n] } -> std::convertible_to<typename C::const_reference>;
        } /*&& RandomAccessIterator<typename C::const_iterator>*/;

    template <typename C>
    concept MutableRandomAccessContainerSpan =
        MutableReversibleContainerSpan<C> && RandomAccessContainerSpan<C> &&
        requires(C container, typename C::size_type n) {
            typename C::reference;
            { container[n] } -> std::convertible_to<typename C::reference>;
        } && MutableRandomAccessIterator<typename C::iterator> &&
        MutableRandomAccessIterator<typename C::reverse_iterator>;

    template <typename C>
    concept ConstSpanLike = MutableRandomAccessContainerSpan<C> || RandomAccessContainer<C>;
} // namespace solar
