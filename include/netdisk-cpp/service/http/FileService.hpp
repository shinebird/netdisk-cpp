#pragma once

#include "netdisk-cpp/data/ShareableFile.hpp"

#include <expected>
#include <filesystem>
#include <vector>

namespace netdisk::service::http
{
    auto getShareableFiles(const std::expected<std::filesystem::path, std::string>& path)
        -> std::vector<data::ShareableFile>;
}
