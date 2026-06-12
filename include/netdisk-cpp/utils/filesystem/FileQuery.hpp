#pragma once

#include <filesystem>
#include <optional>

namespace netdisk::utils::filesystem
{
    auto getFileSize(const std::filesystem::path& path) -> std::optional<std::uintmax_t>;
    auto isRegularFile(const std::filesystem::path& path) -> std::optional<bool>;
    auto isDirectory(const std::filesystem::path& path) -> std::optional<bool>;
    auto lastWriteTime(const std::filesystem::path& path)
        -> std::optional<std::int64_t>;
} // namespace netdisk::utils::filesystem
