#pragma once

#include "netdisk-cpp/data/ShareableFile.hpp"

#include <expected>
#include <filesystem>
#include <flat_set>
#include <string_view>
#include <vector>

namespace netdisk::service::http
{
    auto getShareableFiles(const std::expected<std::filesystem::path, std::string>& path)
        -> std::vector<data::ShareableFile>;

    auto checkFileExists(const std::filesystem::path& path) -> bool;

    auto batchDownloadLinks(const std::filesystem::path& file_path, std::string_view username,
                            std::string_view token, std::string_view domain, std::uint16_t port)
        -> std::vector<std::string>;

    auto moveFileCmd(const std::vector<std::filesystem::path>& paths,
                     const std::flat_set<std::filesystem::path>& dirs) -> std::vector<std::string>;

    auto pathMapping(const std::filesystem::path& file_path, const std::filesystem::path& path,
                     std::vector<std::filesystem::path>& paths,
                     std::flat_set<std::filesystem::path>& dirs) -> void;
} // namespace netdisk::service::http
