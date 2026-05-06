#include <algorithm>
#include <string_view>


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

            auto operator()(const std::string_view& lhs, const std::string_view& rhs) const -> bool
            {
                return std::ranges::lexicographical_compare(
                    lhs, rhs, [](char l, char r) { return std::tolower(l) < std::tolower(r); });
            }
    };
} // namespace netdisk::utils::string
