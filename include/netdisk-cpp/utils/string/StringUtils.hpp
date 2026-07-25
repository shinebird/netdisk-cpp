#include <algorithm>
#include <string_view>
#include <vector>

namespace netdisk::utils::string
{
    struct Hash
    {
            using is_transparent = void; // Enables heterogeneous operations.

            auto operator()(std::string_view sv) const -> std::size_t
            {
                std::hash<std::string_view> hasher;
                return hasher(sv);
            }
    };

    struct CaseInsensitiveCompare
    {
            using is_transparent = void; // Enables heterogeneous operations.

            auto operator()(std::string_view lhs, std::string_view rhs) const -> bool
            {
                return std::ranges::lexicographical_compare(
                    lhs, rhs, [](char l, char r) { return std::tolower(l) < std::tolower(r); });
            }
    };

    auto joinWithNewline(const std::vector<std::string>& lines) -> std::string;

    auto isAscii(std::string_view str) -> bool;
} // namespace netdisk::utils::string
