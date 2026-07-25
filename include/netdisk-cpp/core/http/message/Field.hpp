#pragma once

#include "netdisk-cpp/utils/concept/Common.hpp"

namespace netdisk::core::http::message
{
    template <utils::reflect::StringLike N, utils::reflect::StringLike V> struct BasicField
    {
            N name_;
            V value_;

            // 从任意可转换的 BasicField 构造
            template <typename N2, typename V2>
                requires std::convertible_to<N2, N> && std::convertible_to<V2, V>
            BasicField(const BasicField<N2, V2>& other) : name_(other.name), value_(other.value)
            {
            }

            // 保留默认构造
            BasicField() = default;
            // 保留聚合式构造
            BasicField(N n, V v) : name_(std::move(n)), value_(std::move(v)) {}
    };

    using FieldView = BasicField<std::string_view, std::string_view>;
    using Field = BasicField<std::string, std::string>;

    template <typename T>
    concept FieldLike = requires(const T& f) {
        requires utils::reflect::StringLike<std::remove_cvref_t<decltype(f.name_)>>;
        requires utils::reflect::StringLike<std::remove_cvref_t<decltype(f.value_)>>;
    };

} // namespace netdisk::core::http::message
