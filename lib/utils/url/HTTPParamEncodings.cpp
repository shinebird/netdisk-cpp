#include "netdisk-cpp/utils/url/HTTPParamEncodings.hpp"
#include "netdisk-cpp/utils/string/StringUtils.hpp"

#include <boost/beast/core/detail/base64.hpp>
#include <boost/url.hpp>
#include <boost/url/decode_view.hpp>
#include <boost/url/encode.hpp>
#include <boost/url/rfc/unreserved_chars.hpp>

#include <format>

namespace netdisk::utils::url
{
    auto decodeBase64(std::string_view b64_input) -> std::optional<std::string>
    {
        // Step 1: Base64 解码（Beast 直接支持）
        std::size_t max_decoded = boost::beast::detail::base64::decoded_size(b64_input.size());
        std::string intermediate;
        intermediate.resize(max_decoded);

        auto [written, read] = boost::beast::detail::base64::decode(
            intermediate.data(), b64_input.data(), b64_input.size());

        if (read == 0 && !b64_input.empty())
        {
            return std::nullopt; // Base64 解码失败
        }

        intermediate.resize(written);
        // intermediate 现在是 encodeURIComponent 的输出，如 "E3C%2F"

        // Step 2: Percent-decode 还原为原始字符串
        std::string result;
        result.reserve(intermediate.size());
        const auto decode_view = boost::urls::decode_view(intermediate);

        return std::string(decode_view.begin(), decode_view.end());
    }

    auto encodeBase64(std::string_view utf8_input) -> std::optional<std::string>
    {
        decltype(boost::urls::unreserved_chars) js_safe = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                                          "abcdefghijklmnopqrstuvwxyz"
                                                          "0123456789"
                                                          "-._~!*'()";

        auto encoded = boost::urls::encode(utf8_input, js_safe);

        std::string b64;
        b64.resize(boost::beast::detail::base64::encoded_size(encoded.size()));

        auto written = boost::beast::detail::base64::encode(b64.data(),     // dest buffer
                                                            encoded.data(), // src data
                                                            encoded.size()  // src length
        );

        b64.resize(written); // encoded_size 已是精确值，此行为安全冗余
        return b64;
    }

    namespace rfc5987
    {
        namespace
        {
            constexpr auto isAttrChar(unsigned char c) noexcept -> bool
            {
                return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
                       c == '!' || c == '#' || c == '$' || c == '&' || c == '+' || c == '-' ||
                       c == '.' || c == '^' || c == '_' || c == '`' || c == '|' || c == '~';
            }

            consteval auto makeHexTable()
            {
                // 256 × 3 bytes: 每个字节预计算 "%XX"
                std::array<char, 256 * 3> table{};
                for (int i = 0; i < 256; ++i)
                {
                    table[i * 3 + 0] = '%';
                    table[i * 3 + 1] = "0123456789ABCDEF"[i >> 4];
                    table[i * 3 + 2] = "0123456789ABCDEF"[i & 0x0F];
                }
                return table;
            }

            constexpr auto hex_table = makeHexTable();
        } // namespace
    } // namespace rfc5987

    auto encodeRFC5987(std::string_view utf8_filename) -> std::string
    {
        std::string result;
        result.reserve(utf8_filename.size() * 3);
        for (unsigned char c : utf8_filename)
        {
            if (rfc5987::isAttrChar(c))
            {
                result += static_cast<char>(c);
            }
            else
            {
                result.append(&rfc5987::hex_table[c * 3], 3);
            }
        }
        return result;
    }

    auto encodeContentDispositionFileName(std::string_view utf8_filename,
                                          std::string_view fallback_filename) -> std::string
    {
        std::string_view real_fallback_filename;
        const auto encoded_filename = encodeRFC5987(utf8_filename);
        if (fallback_filename.empty())
        {
            if (!utils::string::isAscii(utf8_filename))
            {
                real_fallback_filename = encoded_filename;
            }
            else
            {
                real_fallback_filename = utf8_filename;
            }
        }
        else
        {
            real_fallback_filename = fallback_filename;
        }
        return std::format("filename=\"{}\"; filename*=UTF-8''{}", real_fallback_filename,
                           encoded_filename);
    }
} // namespace netdisk::utils::url
