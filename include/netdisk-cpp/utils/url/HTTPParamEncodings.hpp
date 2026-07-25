#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace netdisk::utils::url
{
    /// @brief 用于解码：以“原始字符串->encodeURIComponent->window.btoa”方式生成的字符串
    /// @param[in] b64_input 以“原始字符串->encodeURIComponent->window.btoa”方式生成的字符串
    auto decodeBase64(std::string_view b64_input) -> std::optional<std::string>;

    /// @brief 用于编码：以“原始字符串->encodeURIComponent->window.btoa”方式生成的字符串
    /// @param[in] utf8_input 原始字符串
    auto encodeBase64(std::string_view utf8_input) -> std::optional<std::string>;

    auto encodeRFC5987(std::string_view utf8_filename) -> std::string;

    auto encodeContentDispositionFileName(std::string_view utf8_filename,
                                          std::string_view fallback_filename = "") -> std::string;
} // namespace netdisk::utils::url
