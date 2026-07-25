#include "netdisk-cpp/utils/string/StringUtils.hpp"

#include <eve/eve.hpp>
#include <eve/module/algo.hpp>
#include <eve/module/core.hpp>

namespace netdisk::utils::string
{
    auto joinWithNewline(const std::vector<std::string>& lines) -> std::string
    {
        if (lines.empty())
        {
            return {};
        }

        // 1. 预计算总长度（与之前相同，这一步无法省略）
        size_t total = lines.size(); // N 个 '\n'
        for (const auto& line : lines)
        {
            total += line.size();
        }

        // 2. 一次性分配 + 直接填充，零额外检查
        std::string result;
        result.resize_and_overwrite(total,
                                    [&lines](char* buf, size_t n) -> size_t
                                    {
                                        char* p = buf;
                                        for (const auto& str : lines)
                                        {
                                            std::memcpy(p, str.data(), str.size());
                                            p += str.size();
                                            *(p++) = '\n';
                                        }
                                        return n; // 返回实际写入长度（此处 == n）
                                    });

        return result;
    }

    auto isAscii(std::string_view str) -> bool
    {
        auto first_non_ascii = eve::algo::find_if(str,
                                                  [](auto c) -> auto
                                                  {
                                                      auto uc =
                                                          eve::convert(c, eve::as<std::uint8_t>{});
                                                      return uc > 0x7F;
                                                  });

        return first_non_ascii == str.end();
    }
} // namespace netdisk::utils::string
