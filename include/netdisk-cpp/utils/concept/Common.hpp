#pragma once

#include <concepts>
#include <string>
#include <string_view>
#include <type_traits>

#include <netdisk-cpp/utils/Config.h>

#ifdef __CUDACC__
    #include <thrust/tuple.h>
#endif

namespace netdisk::utils::reflect
{
    template <typename T>
    concept Number = std::is_arithmetic_v<std::remove_cvref_t<T>>;

    template <typename T>
    concept Pointer = std::is_pointer_v<std::remove_cvref_t<T>>;

    template <typename T>
    concept PointerLike = requires(T value) { *value; };

    template <typename T>
    concept Struct = std::is_aggregate_v<std::remove_cvref_t<T>>;

    template <typename T>
    concept ClassWithHasValue = requires(T value) {
        { value.hasValue() } -> std::same_as<bool>;
    };

    template <class T>
    concept ClassHasName = requires {
        { T::name() } -> std::same_as<std::string_view>;
    };

    template <typename T, typename K>
    concept ClassWithContains = requires(T value, K key) {
        { value.contains(key) } -> std::same_as<bool>;
    };

    template <typename T>
    concept ClassWithEmpty = requires(T value) {
        { value.empty() } -> std::same_as<bool>;
    };

    template <typename T>
    concept ClassWithToString = requires(T value) {
        { value.toString() } -> std::same_as<std::string>;
    };

    template <typename T>
    concept ClassWithFromString = requires(const std::string& input) {
        { T::fromString(input) } -> std::same_as<T>;
    };

    template <typename T>
    concept StringLike = std::is_same_v<std::string, T> || std::is_same_v<std::string_view, T>;

    template <typename T>
    concept Enum = std::is_enum_v<T>;

    template <typename T>
    concept ToStringConvertible = ClassWithToString<T> || StringLike<T> || Number<T> || Enum<T>;

    template <typename T>
    concept FromStringConvertible = ClassWithFromString<T> || StringLike<T> || Number<T> || Enum<T>;

    template <typename T>
    concept Assignable = requires(T a, T b) {
        { a = b };
    };

    template <typename T>
    concept EqualityComparable = requires(T a, T b) {
        { a == b } -> std::same_as<bool>;
        { a != b } -> std::same_as<bool>;
    };

    template <typename T>
    concept Comparable = requires(T a, T b) {
        { a < b } -> std::same_as<bool>;
        { a > b } -> std::same_as<bool>;
        { a <= b } -> std::same_as<bool>;
        { a >= b } -> std::same_as<bool>;
    };

    template <typename T>
    concept Aggregate = std::is_aggregate_v<T>;

    template <typename T>
    concept TupleLike = requires(T a) {
        std::get<0>(a);
        std::tuple_size_v<std::remove_cvref_t<T>>;
    };

#ifdef __CUDACC__
    template <typename T>
    concept ThrustTupleLike = requires(T a) {
        thrust::get<0>(a);
        thrust::tuple_size<std::remove_cvref_t<T>>::value;
    };
#endif

    template <typename T>
    concept PairLike = requires(T a) {
        TupleLike<T>;
        std::tuple_size_v<std::remove_cvref_t<T>> == 2ULL;
    };

#ifdef __CUDACC__
    template <typename T>
    concept Getable = TupleLike<T> || Struct<T> || ThrustTupleLike<T>;
#else
    template <typename T>
    concept Getable = TupleLike<T> || Struct<T>;
#endif

} // namespace solar
