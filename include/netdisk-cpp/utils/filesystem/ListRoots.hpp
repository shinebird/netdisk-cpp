#pragma once

#include <filesystem>
#include <vector>

namespace netdisk::utils::filesystem
{
    auto listRoots() -> std::vector<std::filesystem::path>;
}
